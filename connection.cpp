#include "connection.h"
#include "context.h"
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include "network.h"
#include <string.h>
#include <assert.h>
#include "common/utils.h"
#include "worker.h"

namespace fantuan
{
Connection::Connection(Worker* worker, int sockfd, const ConnectionHandler& handler, bool et) :
    m_sockfd(sockfd),
    m_Context(new Context(worker, sockfd)),
    m_Handler(handler),
    m_State(CONNECTING),
    m_Worker(worker),
    m_et(et)
{
    bzero(&m_InputBuffer, sizeof(m_InputBuffer));
    ContextHandler contextHandler;
    contextHandler.m_ReadHandler = [=](){this->handleRead();};
    contextHandler.m_WriteHandler = [=](){this->handleWrite();};
    contextHandler.m_ErrorHandler = [=](){this->handleError();};
    contextHandler.m_CloseHandler = [=](){this->handleClose();};
    m_Context->setHandler(contextHandler);
    network::setKeepAlive(m_sockfd, true);
}

Connection::~Connection()
{
    assert(m_State == DISCONNECTED);
    SAFE_DELETE(m_Context);
}

void Connection::handleRead()
{
    if (m_State != CONNECTED) return;
    ssize_t count, n = 0;
    do
    {
        count = network::read(m_sockfd, m_InputBuffer+n, sizeof(m_InputBuffer)-n);
        if (count < 0)
        {
            // another signal captured caused this error, need to try again
            if (errno == EINTR)
                continue;
            // non-block IO means recv buffer is empty
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                // EPIPE or ECONNREST or others
                assert(false && "read error");
                handleError();
                return;
            }
        }
        if (count == 0)
        {
            DEBUG("sock=%d, leave\n", m_sockfd);
            m_Context->setActiveClose();
            return;
        }
        n+=count;
        if (m_et && n == sizeof(m_InputBuffer))
        {
            DEBUG("sock=%d, read full: %ld\n", m_sockfd, n);
            // reach buffer maxsize, need to notify user, et only
            if (m_Handler.m_OnData)
            {
                m_Handler.m_OnData(this, (uint16_t)n, m_InputBuffer);
            }
            bzero(&m_InputBuffer, sizeof(m_InputBuffer));
            n = 0;
        }
    } while (n<sizeof(m_InputBuffer));
    
    if (n > 0)
    {
        TRACE("sock=%d, read: %ld\n", m_sockfd, n);
        if (m_Handler.m_OnData)
        {
            m_Handler.m_OnData(this, (uint16_t)n, m_InputBuffer);
        }
        // TEST SEND FLOW
        if (count > 0 || errno == EAGAIN)
            send(m_InputBuffer, n);
    }
}

void Connection::handleWrite()
{
    if (m_State != CONNECTED) return;
    if (m_Context->isWriting())
    {
        while (m_OutputBuffer.pendingBytes() != 0)
        {
            ssize_t count = network::write(m_sockfd, m_OutputBuffer.peek(), m_OutputBuffer.pendingBytes());
            if (count >= 0)
            {
                m_OutputBuffer.retrieve(count);
                DEBUG("sock=%d, handleWrite: %ld\n", m_sockfd, count);
                if (m_OutputBuffer.pendingBytes() == 0)
                {
                    if (!m_et)
                    {
                        DEBUG("sock=%d, disablewrite\n", m_sockfd);
                        m_Context->disableWriting();
                    }
                    // TODO: write complete callback
                    if (m_State == DISCONNECTING)
                    {
                        shutdown();
                    }
                    break;
                }
                if (!m_et)
                {
                    // lt mode, epollout is always notified until disablewriting
                    // so not necessary to loop write here
                    // but et mode, epollout only notified once.
                    break;
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // here means send buffer is full
                    break;
                }
                // TODO: no error handling?
                assert(false && "write error");
            }
        }
    }
    else
    {
        // TODO: trace no writing
    }
}

void Connection::handleClose()
{
    assert(m_State == CONNECTED || m_State == DISCONNECTING);
    m_State = DISCONNECTED;
    m_Context->disableAll();
    if (m_Handler.m_OnConnection)
    {
        m_Handler.m_OnDisconnected(this);
    }
    m_RemoveConnectionHandler(this);
}

void Connection::handleError()
{
    int err = network::getsockerror(m_sockfd);
    ERROR("sock=%d, network error, err=%d\n", m_sockfd, err);
    //assert(false && "network error");
}

void Connection::shutdown()
{
    if (m_State == CONNECTED)
    {
        m_State = DISCONNECTING;
        if (!m_Context->isWriting())
        {
            network::shutdownWR(m_sockfd);
        }
    }
}

void Connection::send(const void* data, uint32_t len)
{
    if (m_State != CONNECTED) return;
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fatalError = false;
    if ((!m_et && !m_Context->isWriting() && m_OutputBuffer.pendingBytes() == 0) ||
        (m_et && m_OutputBuffer.pendingBytes() == 0))
    {
        nwrote = network::write(m_sockfd, data, len);
        if (nwrote >= 0)
        {
            TRACE("sock=%d, write: %ld\n", m_sockfd, nwrote);
            remaining = len - nwrote;
            if (remaining == 0)
            {
                // TODO: write complete callback
                return;
            }
        }
        else
        {
            nwrote = 0;
            // TODO: what about EINTR
            // EAGAIN means send buffer is full
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                ERROR("sock=%d, send error: %d\n", m_sockfd, errno);
                //assert(false && "send error");
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    fatalError = true;
                }
            }
        }
    }
    if (fatalError)
    {
        handleError();
    }
    else if (remaining > 0)
    {
        DEBUG("sock=%d, avail=%d, remaining=%d, sentIndex=%d\n", m_sockfd, 
            (int)m_OutputBuffer.availBytes(), (int)remaining, (int)m_OutputBuffer.getSentIndex());
        m_OutputBuffer.append((char*)data+nwrote, remaining);
        if (!m_et && !m_Context->isWriting())
        {
            DEBUG("sock=%d, enableWriting\n", m_sockfd);
            m_Context->enableWriting();
        }
    }
}

void Connection::connectEstablished()
{
    assert(m_State == CONNECTING);
    TRACE("sock=%d, connectEstablished\n", m_sockfd);
    m_State = CONNECTED;
    m_Context->enableReading(m_et);
    if (m_et) m_Context->enableWriting(m_et);
    if (m_Handler.m_OnConnection)
    {
        m_Handler.m_OnConnection(this);
    }
}

void Connection::connectDestroyed()
{
    if (m_State == CONNECTED)
    {
        m_State = DISCONNECTED;
        m_Context->disableAll();
    }
    m_Worker->removeContext(m_Context);
    DEBUG("sock=%d, connection destroyed\n", m_sockfd);
}


}
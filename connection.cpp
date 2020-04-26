#include "connection.h"
#include "context.h"
#include "utils.h"
#include "worker.h"
#include "network.h"
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

namespace fantuan
{
Connection::Connection(Worker* worker, int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et) :
    m_worker(worker),
    m_sockfd(sockfd),
    m_sockaddr(addr),
    m_context(new Context(worker, sockfd)),
    m_handler(handler),
    m_state(CONNECTING),
    m_et(et)
{
    bzero(&m_inputBuffer, sizeof(m_inputBuffer));
    m_context->setReadHandler([=](int64_t time){this->_handleRead(time);});
    m_context->setWriteHandler([=](){this->_handleWrite();});
    m_context->setErrorHandler([=](){this->_handleError();});
    m_context->setCloseHandler([=](){this->_handleClose();});
    network::setKeepAlive(m_sockfd, true);
}

Connection::~Connection()
{
    assert(m_state == DISCONNECTED);
    SAFE_DELETE(m_context);
}

void Connection::shutdown()
{
    if (m_state == CONNECTED)
    {
        m_state = DISCONNECTING;
        if (!m_context->isWriting())
        {
            network::shutdown(m_sockfd);
        }
    }
}

void Connection::send(const void* data, uint32_t len)
{
    if (m_state != CONNECTED) return;
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fatalError = false;
    if ((!m_et && !m_context->isWriting() && m_outputBuffer.pendingBytes() == 0) ||
        (m_et && m_outputBuffer.pendingBytes() == 0))
    {
        nwrote = network::write(m_sockfd, data, len);
        if (nwrote >= 0)
        {
            TRACE("sock=%d, write: %ld\n", m_sockfd, nwrote);
            remaining = len - nwrote;
            if (remaining == 0)
            {
                if (m_handler.m_sendData)
                {
                    m_handler.m_sendData(this);
                }
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
        _handleError();
    }
    else if (remaining > 0)
    {
        DEBUG("sock=%d, avail=%d, remaining=%d, sentIndex=%d\n", m_sockfd, 
            (int)m_outputBuffer.availBytes(), (int)remaining, (int)m_outputBuffer.getSentIndex());
        m_outputBuffer.append((char*)data+nwrote, remaining);
        if (!m_et && !m_context->isWriting())
        {
            DEBUG("sock=%d, enableWriting\n", m_sockfd);
            m_context->enableWriting();
        }
    }
}

void Connection::connectEstablished()
{
    assert(m_state == CONNECTING);
    TRACE("sock=%d, connectEstablished\n", m_sockfd);
    m_state = CONNECTED;
    m_context->enableReading(m_et);
    if (m_et) m_context->enableWriting(m_et);
    if (m_handler.m_onConnection)
    {
        m_handler.m_onConnection(this);
    }
}

void Connection::connectDestroyed()
{
    if (m_state == CONNECTED)
    {
        m_state = DISCONNECTED;
        m_context->disableAll();
    }
    m_worker->removeContext(m_context);
    DEBUG("sock=%d, connection destroyed\n", m_sockfd);
}

void Connection::_handleRead(time_t time)
{
    if (m_state != CONNECTED) return;
    ssize_t count, n = 0;
    do
    {
        count = network::read(m_sockfd, m_inputBuffer+n, sizeof(m_inputBuffer)-n);
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
                _handleError();
                return;
            }
        }
        if (count == 0)
        {
            DEBUG("sock=%d, leave\n", m_sockfd);
            m_context->setActiveClose();
            return;
        }
        n+=count;
        if (m_et && n == sizeof(m_inputBuffer))
        {
            DEBUG("sock=%d, read full: %ld\n", m_sockfd, n);
            // reach buffer maxsize, need to notify user, et only
            if (m_handler.m_onData)
            {
                m_handler.m_onData(this, (uint16_t)n, m_inputBuffer);
            }
            bzero(&m_inputBuffer, sizeof(m_inputBuffer));
            n = 0;
        }
    } while (n < (ssize_t)sizeof(m_inputBuffer));
    
    if (n > 0)
    {
        TRACE("sock=%d, read: %ld\n", m_sockfd, n);
        if (m_handler.m_onData)
        {
            m_handler.m_onData(this, (uint16_t)n, m_inputBuffer);
        }
        // TEST SEND FLOW
        if (count > 0 || errno == EAGAIN)
            send(m_inputBuffer, n);
    }
}

void Connection::_handleWrite()
{
    if (m_state != CONNECTED) return;
    if (m_context->isWriting())
    {
        while (m_outputBuffer.pendingBytes() != 0)
        {
            ssize_t count = network::write(m_sockfd, m_outputBuffer.peek(), m_outputBuffer.pendingBytes());
            if (count >= 0)
            {
                m_outputBuffer.retrieve(count);
                DEBUG("sock=%d, handleWrite: %ld\n", m_sockfd, count);
                if (m_outputBuffer.pendingBytes() == 0)
                {
                    if (!m_et)
                    {
                        DEBUG("sock=%d, disablewrite\n", m_sockfd);
                        m_context->disableWriting();
                    }
                    // TODO: write complete callback
                    if (m_state == DISCONNECTING)
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

void Connection::_handleClose()
{
    assert(m_state == CONNECTED || m_state == DISCONNECTING);
    m_state = DISCONNECTED;
    m_context->disableAll();
    if (m_handler.m_onDisconnected)
    {
        m_handler.m_onDisconnected(this);
    }
    m_removeConnectionHandler(this);
}

void Connection::_handleError()
{
    int err = network::getsockerror(m_sockfd);
    ERROR("sock=%d, network error, err=%d\n", m_sockfd, err);
    //assert(false && "network error");
}


}
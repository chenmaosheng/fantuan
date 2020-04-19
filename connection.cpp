#include "connection.h"
#include "context.h"
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include "network.h"
#include <string.h>
#include <assert.h>

namespace fantuan
{
Connection::Connection(int sockfd, Acceptor* acceptor, const ConnectionHandler& handler) :
    m_sockfd(sockfd),
    m_Acceptor(acceptor),
    m_Context(new Context(sockfd)),
    m_Handler(handler),
    m_State(CONNECTING)
{
    bzero(&m_InputBuffer, sizeof(m_InputBuffer));
    ContextHandler contextHandler;
    contextHandler.m_ReadHandler = [=](){this->handleRead();};
    contextHandler.m_WriteHandler = [=](){this->handleWrite();};
    contextHandler.m_ErrorHandler = [=](){this->handleError();};
    contextHandler.m_CloseHandler = [=](){this->handleClose();};
    contextHandler.m_UpdateContextHandler = [=](Context* context){this->m_Acceptor->updateContext(context);};
    m_Context->setHandler(contextHandler);
    network::setKeepAlive(m_sockfd, true);
}

Connection::~Connection()
{
    assert(m_State == DISCONNECTED);
}

void Connection::handleRead()
{
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
            printf("leave\n");
            handleClose();
            return;
        }
        n+=count;
    } while (n<sizeof(m_InputBuffer));
    
    if (n > 0)
    {
        printf("read: %ld\n", n);
        m_Handler.m_OnData(this, (uint16_t)n, m_InputBuffer);
        // TEST SEND FLOW
        if (count > 0 || errno == EAGAIN)
            send(m_InputBuffer, n);
    }
}

void Connection::handleWrite()
{
    printf("ready to handle write\n");
    if (m_Context->isWriting())
    {
        ssize_t count = network::write(m_sockfd, m_OutputBuffer.peek(), m_OutputBuffer.pendingBytes());
        if (count > 0)
        {
            m_OutputBuffer.retrieve(count);
            printf("handleWrite: %ld\n", count);
            if (m_OutputBuffer.pendingBytes() == 0)
            {
                printf("disablewrite\n");
                m_Context->disableWriting();
                // TODO: write complete callback
                if (m_State == DISCONNECTING)
                {
                    shutdown();
                }
            }
        }
        else
        {
            // TODO: no error handling?
            assert(false && "write error");
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
    m_Context->disableWriting();
    m_Context->disableReading();
    m_Handler.m_OnDisconnected(this);
    m_CloseHandler(this);
}

void Connection::handleError()
{
    int err = network::getsockerror(m_sockfd);
    printf("network error, err=%d\n", err);
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
    ssize_t nwrote = 0, wd = 0;
    size_t remaining = len;
    bool fatalError = false;
    if (!m_Context->isWriting() && m_OutputBuffer.pendingBytes() == 0)
    {
        while ((wd = network::write(m_sockfd, (char*)data+nwrote, remaining)) > 0)
        {
            nwrote += wd;
            remaining -= wd;
        }
        if (nwrote > 0) printf("write: %ld\n", nwrote);
        if (remaining == 0)
        {
            // TODO: write complete callback
            return;
        }
        if (wd <= 0)
        {
            // TODO: what about EINTR
            // EAGAIN means send buffer is empty
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                printf("send error: %d\n", errno);
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
        m_OutputBuffer.append((char*)data+nwrote, remaining);
        if (!m_Context->isWriting())
        {
            printf("enableWriting\n");
            m_Context->enableWriting();
        }
    }
}

void Connection::connectEstablished()
{
    assert(m_State == CONNECTING);
    m_State = CONNECTED;
    m_Context->enableReading();
    m_Handler.m_OnConnection(this);
}

void Connection::connectDestroyed()
{
    if (m_State == CONNECTED)
    {
        m_State = DISCONNECTED;
        m_Acceptor->removeContext(m_Context);
        // TODO: disconnection handler
    }
    printf("close socket\n");
    network::close(m_sockfd);
}


}
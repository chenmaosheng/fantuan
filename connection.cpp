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
    m_Handler(handler)
{
    bzero(&m_InputBuffer, sizeof(m_InputBuffer));
    bzero(&m_OutputBuffer, sizeof(m_OutputBuffer));
    ContextHandler contextHandler;
    contextHandler.m_ReadHandler = [=](){this->handleRead();};
    contextHandler.m_WriteHandler = [=](){this->handleWrite();};
    contextHandler.m_UpdateContextHandler = [=](Context* context){this->m_Acceptor->updateContext(context);};
    m_Context->setHandler(contextHandler);
    network::setKeepAlive(m_sockfd, true);
}

Connection::~Connection()
{

}

void Connection::handleRead()
{
    ssize_t count, n = 0;
    do
    {
        count = network::read(m_sockfd, m_InputBuffer+n, sizeof(m_InputBuffer)-n);
        if (count < 0)
        {
            if (errno == EINTR)
                break;
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
                continue;
            else
            {
                // EPIPE or ECONNREST or others
                assert(false && "read error");
                handleError();
                break;
            }
        }
        if (count == 0)
        {
            printf("leave\n");
            handleClose();
            break;
        }
        n+=count;
    } while (n<sizeof(m_InputBuffer));
    
    if (n > 0)
    {
        printf("read: %ld\n", count);
        send(m_InputBuffer, strlen(m_InputBuffer));
    }
}

void Connection::handleWrite()
{
    if (m_Context->isWriting())
    {
        printf("disableWriting\n");
        ssize_t count = network::write(m_sockfd, m_OutputBuffer, 1024);
        if (count > 0)
        {
            printf("handleWrite: %ld\n", count);
            m_Context->disableWriting();
        }
        else
        {
            assert(false && "write error");
        }
    }
}

void Connection::handleClose()
{
    m_Context->disableWriting();
    m_Context->disableReading();
    m_CloseHandler(this);
}

void Connection::handleError()
{
    int err = network::getsockerror(m_sockfd);
    assert(false && "network error");
    handleClose();
}

void Connection::send(const void* data, uint32_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fatalError = false;
    if (!m_Context->isWriting())
    {
        nwrote = network::write(m_sockfd, data, len);
        if (nwrote >= 0)
        {
            printf("write: %ld\n", nwrote);
            remaining = len - nwrote;
            if (remaining == 0)
            {
                // TODO: write complete callback
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                assert(false && "send error");
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    fatalError = true;
                }
            }
        }
    }

    if (!fatalError && remaining > 0)
    {
        strncpy(m_OutputBuffer, (char*)data+nwrote, remaining);
        if (!m_Context->isWriting())
        {
            printf("enableWriting\n");
            m_Context->enableWriting();
        }
    }
}

void Connection::connectEstablished()
{
    m_Context->enableReading();
}

void Connection::connectDestroyed()
{
    // TODO:
    m_Acceptor->removeContext(m_Context);
    network::close(m_sockfd);
}


}
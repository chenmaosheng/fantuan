#include "connection.h"
#include "context.h"
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include "network.h"
#include <string.h>

namespace fantuan
{
Connection::Connection(int sockfd, Acceptor* acceptor) :
    m_sockfd(sockfd),
    m_Acceptor(acceptor),
    m_Context(new Context(sockfd))
{
    m_Context->setEvents(EPOLLIN);
    bzero(&m_InputBuffer, sizeof(m_InputBuffer));
    bzero(&m_OutputBuffer, sizeof(m_OutputBuffer));
    ContextHandler handler;
    handler.m_ReadHandler = [=](){this->handleRead();};
    handler.m_WriteHandler = [=](){this->handleWrite();};
    m_Context->setHandler(handler);

}

Connection::~Connection()
{

}

void Connection::handleRead()
{
    ssize_t count;
    count = network::read(m_sockfd, m_InputBuffer, sizeof(m_InputBuffer));
    if (count == -1)
    {
        if (errno != EAGAIN)
        {
            // TODO: fatal error
        }
    }
    else if (count == 0)
    {

    }
    else 
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
            // TODO: fatal error
        }
    }
}

void Connection::send(const void* data, uint32_t len)
{
    ssize_t wrote = 0;
    size_t remaining = len;
    bool fatalError = false;
    if (!m_Context->isWriting())
    {
        wrote = network::write(m_sockfd, data, len);
        if (wrote >= 0)
        {
            printf("write: %ld\n", wrote);
            remaining = len - wrote;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (!m_Context->isWriting())
                {
                    printf("enableWriting\n");
                    m_Context->enableWriting();
                }
            }
            else
            {
                // TODO: fatal error;
            }
        }
    }
}


}
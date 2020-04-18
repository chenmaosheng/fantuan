#include "context.h"
#include "connection.h"
#include <stdio.h>

namespace fantuan
{
Context::Context(int sockfd) :
    m_sockfd(sockfd)
{

}

Context::~Context()
{
    
}

void Context::handleEvent()
{
    if (m_Events & (EPOLLIN || EPOLLERR || EPOLLRDHUP || EPOLLPRI))
    {
        m_handler.m_ReadHandler();
    }
    else if (m_Events & EPOLLOUT)
    {
        m_handler.m_WriteHandler();
    }
    else
    {
        // TODO: fatal error
    }
}

void Context::enableWriting()
{
    m_Events |= EPOLLOUT;
    m_handler.m_UpdateContextHandler(this);
}

void Context::disableWriting()
{
    m_Events &= ~EPOLLOUT;
    m_handler.m_UpdateContextHandler(this);
}

void Context::enableReading()
{
    m_Events |= EPOLLIN;
    m_handler.m_UpdateContextHandler(this);
}

void Context::disableReading()
{
    m_Events &= ~EPOLLIN;
    m_handler.m_UpdateContextHandler(this);
}

}
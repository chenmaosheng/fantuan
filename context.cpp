#include "context.h"
#include "connection.h"
#include <stdio.h>

namespace fantuan
{
Context::Context(int sockfd) :
    m_sockfd(sockfd),
    m_Events(0),
    m_activeEvents(0),
    m_state(NEW)
{

}

Context::~Context()
{
    
}

void Context::handleEvent()
{
    if ((m_activeEvents & EPOLLHUP) && !(m_activeEvents & EPOLLIN))
    {
        m_handler.m_CloseHandler();
    }
    if (m_activeEvents & EPOLLERR)
    {
        m_handler.m_ErrorHandler();
    }
    if (m_activeEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        m_handler.m_ReadHandler();
    }
    if (m_activeEvents & EPOLLOUT)
    {
        m_handler.m_WriteHandler();
    }
}

void Context::enableWriting(bool et)
{
    if (et) m_Events |= EPOLLET;
    m_Events |= EPOLLOUT;
    m_handler.m_UpdateContextHandler(this);
}

void Context::disableWriting()
{
    m_Events &= ~EPOLLOUT;
    m_handler.m_UpdateContextHandler(this);
}

void Context::enableReading(bool et)
{
    if (et) m_Events |= EPOLLET;
    m_Events |= (EPOLLIN | EPOLLPRI);
    m_handler.m_UpdateContextHandler(this);
}

void Context::disableReading()
{
    m_Events &= ~(EPOLLIN | EPOLLPRI);
    m_handler.m_UpdateContextHandler(this);
}

void Context::disableAll()
{
    m_Events = 0;
    m_handler.m_UpdateContextHandler(this);
}

}
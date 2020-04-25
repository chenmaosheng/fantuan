#include "context.h"
#include "connection.h"
#include <stdio.h>
#include "worker.h"

namespace fantuan
{
Context::Context(Worker* worker, int sockfd) :
    m_sockfd(sockfd),
    m_Events(0),
    m_activeEvents(0),
    m_state(NEW),
    m_Worker(worker),
    m_activeClose(false)
{

}

Context::~Context()
{
    
}

void Context::handleEvent()
{
    if (!m_activeClose && (m_activeEvents & EPOLLERR))
    {
        m_handler.m_ErrorHandler();
    }
    if (!m_activeClose && (m_activeEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))) // EPOLLRDHUP means target close or shutdown write
    // EPOLLHUP means self call shutdown RDWR, must not be close, close will invalidate file descriptor
    {
        m_handler.m_ReadHandler();
    }
    if (!m_activeClose && (m_activeEvents & EPOLLOUT))
    {
        m_handler.m_WriteHandler();
    }
    if (!m_activeClose && (m_activeEvents & EPOLLRDHUP))
    {
        m_activeClose = true;
    }
    if (m_activeClose)
    {
        m_handler.m_CloseHandler();
    }
}

void Context::enableWriting(bool et)
{
    if (et) m_Events |= EPOLLET;
    m_Events |= EPOLLOUT;
    _update();
}

void Context::disableWriting()
{
    m_Events &= ~EPOLLOUT;
    _update();
}

void Context::enableReading(bool et)
{
    if (et) m_Events |= EPOLLET;
    m_Events |= (EPOLLIN | EPOLLPRI);
    _update();
}

void Context::disableReading()
{
    m_Events &= ~(EPOLLIN | EPOLLPRI);
    _update();
}

void Context::disableAll()
{
    m_Events = 0;
    _update();
}

void Context::remove()
{
    m_Worker->removeContext(this);
}

void Context::_update()
{
    m_Worker->updateContext(this);
}

}
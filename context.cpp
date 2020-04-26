#include "context.h"
#include "worker.h"
#include "utils.h"

namespace fantuan
{
Context::Context(Worker* worker, int sockfd) :
    m_worker(worker),
    m_sockfd(sockfd),
    m_events(0),
    m_activeEvents(0),
    m_state(NEW),
    m_activeClose(false)
{

}

Context::~Context()
{
    
}

void Context::handleEvent()
{
    if (!m_activeClose && (m_activeEvents & EPOLLERR) && m_errorHandler)
    {
        m_errorHandler();
    }
    if (!m_activeClose && (m_activeEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) && m_readHandler) 
    // EPOLLRDHUP means target close or shutdown write
    // EPOLLHUP means self call shutdown RDWR, must not be close, close will invalidate file descriptor
    {
        m_readHandler(now());
    }
    if (!m_activeClose && (m_activeEvents & EPOLLOUT) && m_writeHandler)
    {
        m_writeHandler();
    }
    if (!m_activeClose && (m_activeEvents & EPOLLRDHUP))
    {
        m_activeClose = true;
    }
    if (m_activeClose)
    {
        m_closeHandler();
    }
}

void Context::remove()
{
    m_worker->removeContext(this);
}

void Context::_update()
{
    m_worker->updateContext(this);
}

}
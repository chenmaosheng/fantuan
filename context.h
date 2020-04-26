#ifndef _H_CONTEXT
#define _H_CONTEXT

#include <sys/epoll.h>
#include "handler.h"

namespace fantuan
{
class Worker;
class Context
{
public:
    enum State { NEW, ADDED, DELETED };
    Context(Worker* worker, int sockfd);
    ~Context();

    void setReadHandler(OnReadEvent handler)
    {
        m_readHandler = std::move(handler);
    }
    void setWriteHandler(OnEvent handler)
    {
        m_writeHandler = std::move(handler);
    }
    void setCloseHandler(OnEvent handler)
    {
        m_closeHandler = std::move(handler);
    }
    void setErrorHandler(OnEvent handler)
    {
        m_errorHandler = std::move(handler);
    }
    void handleEvent();
    int getSockFd() const
    {
        return m_sockfd;
    }

    int getEvents() const
    {
        return m_events;
    }

    void setActiveEvents(int events)
    {
        m_activeEvents = events;
    }

    void setActiveClose()
    {
        m_activeClose = true;
    }

    void enableWriting(bool et = false)
    {
        if (et) m_events |= EPOLLET;
        m_events |= EPOLLOUT;
        _update();
    }

    void disableWriting()
    {
        m_events &= ~EPOLLOUT;
        _update();
    }

    void enableReading(bool et = false)
    {
        if (et) m_events |= EPOLLET;
        m_events |= (EPOLLIN | EPOLLPRI);
        _update();
    }

    void disableReading()
    {
        m_events &= ~(EPOLLIN | EPOLLPRI);
        _update();
    }

    void disableAll()
    {
        m_events = 0;
        _update();
    }
    
    bool isWriting() const
    {
        return m_events & EPOLLOUT;
    }
    bool isReading() const
    {
        return m_events & (EPOLLIN | EPOLLPRI);
    }
    void setState(State state)
    {
        m_state = state;
    }
    State getState() const
    {
        return m_state;
    }
    void remove();

private:
    void _update();

private:
    Worker*     m_worker;
    const int   m_sockfd;
    int         m_events;
    int         m_activeEvents;
    State       m_state;
    bool        m_activeClose;
    OnReadEvent m_readHandler;
    OnEvent     m_writeHandler;
    OnEvent     m_closeHandler;
    OnEvent     m_errorHandler;
};
}

#endif // _H_CONTEXT
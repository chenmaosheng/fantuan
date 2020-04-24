#ifndef _H_CONTEXT
#define _H_CONTEXT

#include <sys/epoll.h>
#include "handler.h"

namespace fantuan
{
class Connection;
class Worker;
class Context
{
public:
    enum State { NEW, ADDED, DELETED};
    Context(Worker* worker, int sockfd);
    ~Context();

    void setHandler(const ContextHandler& handler)
    {
        m_handler = std::move(handler);
    }
    void handleEvent();
    int getSockFd() const
    {
        return m_sockfd;
    }

    int getEvents() const
    {
        return m_Events;
    }

    void setActiveEvents(int events)
    {
        m_activeEvents = events;
    }

    void enableWriting(bool et = false);
    void disableWriting();
    void enableReading(bool et = false);
    void disableReading();
    void disableAll();
    bool isWriting() const
    {
        return m_Events & EPOLLOUT;
    }
    bool isReading() const
    {
        return m_Events & (EPOLLIN | EPOLLPRI);
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
    const int m_sockfd;
    int m_Events;
    int m_activeEvents;
    ContextHandler m_handler;
    State m_state;
    Worker* m_Worker;
};
}

#endif // _H_CONTEXT
#ifndef _H_CONTEXT
#define _H_CONTEXT

#include <sys/epoll.h>
#include "handler.h"

namespace fantuan
{
class Connection;
class Context
{
public:
    Context(int sockfd);
    ~Context();

    void setHandler(const ContextHandler& handler)
    {
        m_handler = handler;
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

    void setEvents(int events)
    {
        m_Events = events;
    }

    void enableWriting();
    void disableWriting();
    void enableReading();
    void disableReading();
    bool isWriting() const
    {
        return m_Events & EPOLLOUT;
    }
    bool isReading() const
    {
        return m_Events & EPOLLIN;
    }

private:
    const int m_sockfd;
    int m_Events;
    ContextHandler m_handler;
};
}

#endif // _H_CONTEXT
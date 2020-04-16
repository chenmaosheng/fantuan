#ifndef _H_CONTEXT
#define _H_CONTEXT

#include <sys/epoll.h>

namespace fantuan
{
class Connection;
class Context
{
public:
    Context(int sockfd, Connection* conn = nullptr);
    ~Context();

    Connection* getConnection() const
    {
        return m_Connection;
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
    bool isWriting() const
    {
        return m_Events & EPOLLOUT;
    }

private:
    const int m_sockfd;
    int m_Events;
    Connection* m_Connection;
};
}

#endif // _H_CONTEXT
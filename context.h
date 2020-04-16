#ifndef _H_CONTEXT
#define _H_CONTEXT

namespace fantuan
{
class Context
{
public:
    Context(int sockfd);
    ~Context();

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

private:
    const int m_sockfd;
    int m_Events;
};
}

#endif // _H_CONTEXT
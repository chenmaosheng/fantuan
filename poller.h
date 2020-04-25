#ifndef _H_POLLER
#define _H_POLLER

#include <vector>
#include <sys/epoll.h>

namespace fantuan
{
class Context;
class Poller
{
public:
    Poller();
    ~Poller();

    void poll(std::vector<Context*>& activeContexts, int timeout=10000); // 10000ms
    void updateContext(Context* context);
    void removeContext(Context* context);

private:
    void _updateContext(int operation, Context* context);

private:
    const static int m_InitEventListSize = 16;
    int m_epollfd;
    std::vector<epoll_event> m_EventList;
};
}

#endif // _H_POLLER
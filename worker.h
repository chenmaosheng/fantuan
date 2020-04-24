#ifndef _H_WORKER
#define _H_WORKER

#include <vector>
#include <functional>
#include <atomic>
#include <sys/epoll.h>
#include <thread>
#include "handler.h"

namespace fantuan
{
class Context;
class Worker
{
public:
    Worker();
    ~Worker();

    void SetPostEventHandler(const PostEventHandler& handler)
    {
        m_PostEventHandler = std::move(handler);
    }
    void loop();
    void quit();

    void poll(int timeout=10000); // 10000ms
    void updateContext(Context* context);
    void removeContext(Context* context);

    void startThread();

private:
    void _updateContext(int operation, Context* context);

private:
    std::atomic_bool m_Quit;
    const static int m_InitEventListSize = 16;
    int m_epollfd;
    std::vector<epoll_event> m_EventList;
    PostEventHandler m_PostEventHandler;
    std::thread* m_Thread;
};
}

#endif // _H_WORKER
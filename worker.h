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
class Poller;
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

    void updateContext(Context* context);
    void removeContext(Context* context);

    void startThread();

private:
    std::atomic_bool m_Quit;
    PostEventHandler m_PostEventHandler;
    std::thread* m_Thread;
    Poller* m_Poller;
    std::vector<Context*> m_ActiveContexts;
};
}

#endif // _H_WORKER
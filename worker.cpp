#include "worker.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include "common/utils.h"
#include "context.h"
#include "poller.h"

struct IgnorePipe
{
IgnorePipe() {::signal(SIGPIPE, SIG_IGN);} // ignore sigpipe, must run before socket initialization
};
IgnorePipe obj;

namespace fantuan
{
Worker::Worker() : 
    m_Quit(false), 
    m_Thread(nullptr),
    m_Poller(new Poller),
    m_ThreadId(std::this_thread::get_id())
{
    
}

Worker::~Worker()
{
    if (m_Thread)
    {
        m_Quit = true;
        m_Thread->join();
        SAFE_DELETE(m_Thread);
    }
    SAFE_DELETE(m_Poller);
}

void Worker::loop()
{
    while (!m_Quit)
    {
        m_ActiveContexts.clear();
        m_Poller->poll(m_ActiveContexts);
        for (auto& context : m_ActiveContexts)
        {
            context->handleEvent();
        }
        std::vector<EventHandler> tmp;
        {
            std::unique_lock<std::mutex> lock(m_PendingHandlerMutex);
            tmp.swap(m_PendingHandlers);
            TRACE("worker=%p, tmp size=%d\n", this, (int)tmp.size());
        }
        for (auto& handler : tmp)
        {
            handler();
        }
    }
}

void Worker::quit()
{
    m_Quit = true;
}

void Worker::run(const EventHandler& handler)
{
    if (isInSameThread())
    {
        handler();
    }
    else
    {
        queue(handler);
    }
}

void Worker::queue(const EventHandler& handler)
{
    std::unique_lock<std::mutex> lock(m_PendingHandlerMutex);
    m_PendingHandlers.push_back(std::move(handler));
    TRACE("worker=%p, size=%d\n", this, (int)m_PendingHandlers.size());
}

void Worker::updateContext(Context* context)
{
    m_Poller->updateContext(context);
}

void Worker::removeContext(Context* context)
{
    m_Poller->removeContext(context);
}

void Worker::startThread()
{
    m_Thread = new std::thread(std::bind(&Worker::loop, this));
    m_ThreadId = m_Thread->get_id();
}

}
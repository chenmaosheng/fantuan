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
    m_Poller(new Poller)
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
            // make sure all events have been handled, then check if connection is already destroyed
            // this should be the better and more graceful behavior
            if (m_PostEventHandler)
            {
                m_PostEventHandler(context->getSockFd());
            }
        }
    }
}

void Worker::quit()
{
    m_Quit = true;
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
}

}
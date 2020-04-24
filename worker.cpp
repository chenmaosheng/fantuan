#include "worker.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include "common/utils.h"
#include "context.h"

struct IgnorePipe
{
IgnorePipe() {::signal(SIGPIPE, SIG_IGN);} // ignore sigpipe, must run before socket initialization
};
IgnorePipe obj;

namespace fantuan
{
Worker::Worker() : 
    m_Quit(false), 
    m_EventList(m_InitEventListSize),
    m_Thread(nullptr)
{
    m_epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if (m_epollfd < 0)
    {
        assert(false && "create epoll failed");
    }
}

Worker::~Worker()
{
    if (m_Thread)
    {
        m_Quit = true;
        m_Thread->join();
        SAFE_DELETE(m_Thread);
    }
    ::close(m_epollfd);
}

void Worker::loop()
{
    while (!m_Quit)
    {
        poll();
    }
}

void Worker::quit()
{
    m_Quit = true;
}

void Worker::poll(int timeout)
{
    int n = epoll_wait(m_epollfd, &*m_EventList.begin(), m_EventList.size(), 10000);
    if (n > 0)
    {
        for (int i = 0; i < n; ++i)
        {
            Context* context = (Context*)m_EventList[i].data.ptr;
            context->setActiveEvents(m_EventList[i].events);
            context->handleEvent();
            // make sure all events have been handled, then check if connection is already destroyed
            // this should be the better and more graceful behavior
            if (m_PostEventHandler)
            {
                m_PostEventHandler(context->getSockFd());
            }
        }
        if (n == m_EventList.size())
        {
            m_EventList.resize(m_EventList.size()*2);
            PRINTF("n=%d, new event list size: %d\n", n, (int)m_EventList.size());
        }
    }
    else if (n == 0)
    {
        // TODO: trace
    }
    else
    {
        if (errno != EINTR)
        {
            assert(false && "epoll wait failed");
        }
    }
}

void Worker::updateContext(Context* context)
{
    int state = context->getState();
    if (state == Context::NEW || state == Context::DELETED)
    {
        // new context
        context->setState(Context::ADDED);
        _updateContext(EPOLL_CTL_ADD, context);
    }
    else
    {
        _updateContext(EPOLL_CTL_MOD, context);
    }
}

void Worker::removeContext(Context* context)
{
    int state = context->getState();
    assert(state == Context::ADDED);
    if (state == Context::ADDED)
    {
        _updateContext(EPOLL_CTL_DEL, context);
    }
    context->setState(Context::NEW);
}

void Worker::_updateContext(int operation, Context* context)
{
    epoll_event event;
    bzero(&event, sizeof(epoll_event));
    event.events = context->getEvents();
    event.data.ptr = context;
    int fd = context->getSockFd();
    if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
    {
        assert(false && "epoll_ctl error");
    }
}

void Worker::startThread()
{
    m_Thread = new std::thread(std::bind(&Worker::loop, this));
}

}
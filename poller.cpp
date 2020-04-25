#include "poller.h"
#include "context.h"
#include <assert.h>
#include "common/utils.h"
#include <strings.h>
#include <unistd.h>

namespace fantuan
{
Poller::Poller() :
    m_EventList(m_InitEventListSize)
{
    m_epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if (m_epollfd < 0)
    {
        assert(false && "create epoll failed");
    }
}

Poller::~Poller()
{
    ::close(m_epollfd);
}

void Poller::poll(std::vector<Context*>& activeContexts, int timeout)
{
    int n = epoll_wait(m_epollfd, &*m_EventList.begin(), m_EventList.size(), timeout);
    if (n > 0)
    {
        for (int i = 0; i < n; ++i)
        {
            Context* context = (Context*)m_EventList[i].data.ptr;
            context->setActiveEvents(m_EventList[i].events);
            activeContexts.push_back(context);
        }
        if (n == m_EventList.size())
        {
            m_EventList.resize(m_EventList.size()*2);
            ALERT("new event list size: %d\n", (int)m_EventList.size());
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

void Poller::updateContext(Context* context)
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

void Poller::removeContext(Context* context)
{
    int state = context->getState();
    assert(state == Context::ADDED);
    if (state == Context::ADDED)
    {
        _updateContext(EPOLL_CTL_DEL, context);
    }
    context->setState(Context::NEW);
}

void Poller::_updateContext(int operation, Context* context)
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

}
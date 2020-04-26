#include "poller.h"
#include "context.h"
#include "utils.h"
#include "network.h"
#include <assert.h>
#include <strings.h>
#include <unistd.h>

namespace fantuan
{
Poller::Poller() :
    m_epollfd(network::createepoll()),
    m_eventList(m_initEventListSize)
{
}

Poller::~Poller()
{
    ::close(m_epollfd);
}

void Poller::poll(std::vector<Context*>& activeContexts, int timeout)
{
    int n = epoll_wait(m_epollfd, &*m_eventList.begin(), m_eventList.size(), timeout);
    if (n > 0)
    {
        for (int i = 0; i < n; ++i)
        {
            Context* context = (Context*)m_eventList[i].data.ptr;
            context->setActiveEvents(m_eventList[i].events);
            activeContexts.push_back(context);
        }
        if (n == (int)m_eventList.size())
        {
            m_eventList.resize(m_eventList.size()*2);
            ALERT("new event list size: %d\n", (int)m_eventList.size());
        }
    }
    else if (n == 0)
    {
        TRACE("no event from epoll_wait, n=%d\n", n);
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
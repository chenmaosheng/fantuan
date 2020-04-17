#include "acceptor.h"
#include "network.h"
#include <stdio.h>
#include "connection.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>

namespace fantuan
{
Acceptor::Acceptor(uint16_t port, bool reuseport) : 
    m_acceptfd(network::createsocket()),
    m_Listening(false),
    m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    m_EventList(16),
    m_AcceptContext(m_acceptfd)
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, reuseport);
    network::bind(m_acceptfd, port);
}

Acceptor::~Acceptor()
{
    ::close(m_epollfd);
}

void Acceptor::listen()
{
    m_Listening = true;
    network::listen(m_acceptfd);
    m_AcceptContext.setEvents(EPOLLIN);
    _updateContext(EPOLL_CTL_ADD, &m_AcceptContext);
}

int Acceptor::handleRead()
{
    int connfd = network::accept(m_acceptfd);
    if (connfd >= 0)
    {
        printf("accept\n");
        // TODO: 
    }
    else
    {
        // TODO: fatal error;
    }
    return connfd;
}

void Acceptor::poll()
{
    int n = epoll_wait(m_epollfd, &*m_EventList.begin(), m_EventList.size(), -1);
    for (int i = 0; i < n; ++i)
    {
        Context* context = (Context*)m_EventList[i].data.ptr;
        if (m_acceptfd == context->getSockFd())
        {
            int fd = handleRead();
            Connection* conn = new Connection(fd, this);
            _updateContext(EPOLL_CTL_ADD, conn->getContext());
        }
        else
        {
            context->handleEvent();
        }
    }
}

void Acceptor::updateContext(Context* context)
{
    _updateContext(EPOLL_CTL_MOD, context);
}

void Acceptor::_updateContext(int operation, Context* context)
{
    epoll_event event;
    bzero(&event, sizeof(epoll_event));
    event.events = context->getEvents();
    event.data.ptr = context;
    int fd = context->getSockFd();
    if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
    {
        // TODO: fatal error
    }
}

}
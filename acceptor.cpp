#include "acceptor.h"
#include "network.h"
#include <stdio.h>
#include "connection.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>

namespace fantuan
{
Acceptor::Acceptor(uint16_t port) : 
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_Listening(false),
    m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    m_EventList(m_InitEventListSize),
    m_AcceptContext(m_acceptfd)
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, true);
    network::bind(m_acceptfd, port);
    ContextHandler handler;
    handler.m_UpdateContextHandler = [=](Context* context){this->updateContext(context);};
    handler.m_ReadHandler = [=](){this->handleRead();};
    m_AcceptContext.setHandler(handler);
}

Acceptor::~Acceptor()
{
    ::close(m_epollfd);
    ::close(m_idlefd);
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
        _newConnection(connfd);
    }
    else
    {
        assert(false && "accept failed");
        // if file descriptor has used up, it'll cause accept failure and return EMFILE error
        // but it doesn't refuse this connection, still in connection queue, and still can 
        // trigger fd read event, this will cause busy loop. Use the following way can refuse
        // connection gracefully
        if (errno == EMFILE)
        {
            ::close(m_idlefd);
            m_idlefd = ::accept(m_acceptfd, NULL, NULL);
            ::close(m_idlefd);
            m_idlefd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
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
            handleRead();
        }
        else
        {
            context->handleEvent();
        }
    }
    if (n == m_eventList.size())
    {
        m_eventList.resize(m_eventList.size()*2);
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
        assert(false && "epoll_ctl error");
    }
}

void Acceptor::_newConnection(int sockfd)
{
     Connection* conn = new Connection(sockfd, this);
     _updateContext(EPOLL_CTL_ADD, conn->getContext());
}

}
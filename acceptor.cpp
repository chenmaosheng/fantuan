#include "acceptor.h"
#include "network.h"
#include <stdio.h>
#include "connection.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

namespace fantuan
{
struct IgnorePipe
{
IgnorePipe() {::signal(SIGPIPE, SIG_IGN);} // ignore sigpipe, must run before socket initialization
};
IgnorePipe obj;

Acceptor::Acceptor(uint16_t port) : 
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_Listening(false),
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

    m_epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if (m_epollfd < 0)
    {
        assert(false && "create epoll failed");
    }
}

Acceptor::~Acceptor()
{
    for (auto& item : m_Connections)
    {
        item.second->connectDestroyed();
    }
    removeContext(&m_AcceptContext);
    ::close(m_epollfd);
    ::close(m_idlefd);
}

void Acceptor::listen()
{
    m_Listening = true;
    network::listen(m_acceptfd);
    m_AcceptContext.enableReading();
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
    if (n > 0)
    {
        for (int i = 0; i < n; ++i)
        {
            Context* context = (Context*)m_EventList[i].data.ptr;
            context->setActiveEvents(m_EventList[i].events);
            context->handleEvent();
        }
        if (n == m_EventList.size())
        {
            m_EventList.resize(m_EventList.size()*2);
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

void Acceptor::updateContext(Context* context)
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

void Acceptor::removeContext(Context* context)
{
    int state = context->getState();
    assert(state == Context::ADDED);
    if (state == Context::ADDED)
    {
        _updateContext(EPOLL_CTL_DEL, context);
    }
    context->setState(Context::NEW);
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
     Connection* conn = new Connection(sockfd, this, m_Handler);
     m_Connections[sockfd] = conn;
     conn->setCloseHandler([=](Connection* conn){this->_removeConnection(conn);});
     conn->connectEstablished();
}

void Acceptor::_removeConnection(Connection* conn)
{
    if (conn)
    {
        m_Connections.erase(conn->getSockfd());
        conn->connectDestroyed();
        // TODO: can't delete connection here, because this function is called by connection itself. 
        // TODO: how to gracefully delete connection
        delete conn;
        conn = nullptr;
        // this is called when connection's handleclose. so after handleclose, you can't call any connection APIs.
    }
}

}
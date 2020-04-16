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
    m_AcceptSocket(network::createsocket()),
    m_Listening(false),
    m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    m_EventList(16)
{
    m_AcceptSocket.setReuseAddr(true);
    m_AcceptSocket.setReusePort(reuseport);
    m_AcceptSocket.bind(port);
}

Acceptor::~Acceptor()
{
    ::close(m_epollfd);
}

void Acceptor::listen()
{
    m_Listening = true;
    m_AcceptSocket.listen();
    m_AcceptEvent.data.fd = m_AcceptSocket.getSockFd();
    m_AcceptEvent.events = EPOLLIN | EPOLLOUT;
    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_AcceptSocket.getSockFd(), &m_AcceptEvent) < 0)
    {
        // TODO: fatal error
    }
}

int Acceptor::handleRead()
{
    int connfd = m_AcceptSocket.accept();
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
        if (m_AcceptSocket.getSockFd() == m_EventList[i].data.fd)
        {
            int fd = handleRead();
            Connection* conn = new Connection(fd, this);
            epoll_event event;
            bzero(&event, sizeof(epoll_event));
            event.data.fd = fd;
            event.events = EPOLLIN | EPOLLOUT;
            if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
            {
                // todo: fatal error
            }
        }
        else
        {
            size_t count;
            char buf[512];
            count = network::read(m_EventList[i].data.fd, buf, sizeof buf);
            if (count == -1)
            {
                if (errno != EAGAIN)
                {
                    // TODO: fatal error
                }
            }
            else if (count == 0)
            {

            }
            else printf("%s\n", buf);
        }
    }
}

}
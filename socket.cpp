#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"
#include <strings.h>
#include <errno.h>
#include "network.h"
#include <netinet/tcp.h>

namespace fantuan
{
Socket::~Socket()
{
    network::close(m_sockfd);
}

void Socket::bind(uint16_t port)
{
    network::bind(m_sockfd, port);
}

void Socket::listen()
{
    network::listen(m_sockfd);
}

int Socket::accept()
{
    sockaddr addr;
    bzero(&addr, sizeof(sockaddr));
    int connfd = network::accept(m_sockfd, &addr);
    if (connfd >= 0)
    {
        // TODO: save addr
    }
    return connfd;
}

void Socket::shutdownWR()
{
    network::shutdownWR(m_sockfd);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0)
    {
        // TODO: fatal error
    }
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        // TODO: fatal error
    }
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        // TODO: fatal error
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
    {
        // TODO: fatal error
    }
}
}

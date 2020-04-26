#include "network.h"
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <stdlib.h>

namespace fantuan
{
namespace network
{
int createsocket()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_IP);
    if (sockfd < 0)
    {
        assert(false && "createsocket failed");
        return -1;
    }
    return sockfd;
}

int createepoll()
{
    int epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollfd < 0)
    {
        assert(false && "create epoll failed");
    }
    return epollfd;
}

int createeventfd()
{
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0)
    {
        assert(false && "create event fd failed");
        abort();
    }
    return eventfd;
}

void bind(int sockfd, uint16_t port)
{
    sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);
    int ret = ::bind(sockfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        assert(false && "bind failed");
    }
}

void listen(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        assert(false && "listen failed");
    }
}

int accept(int sockfd, sockaddr_in& clientAddr)
{
    socklen_t len = (socklen_t)sizeof(sockaddr_in);
    bzero(&clientAddr, sizeof(sockaddr_in));
    int connfd = ::accept4(sockfd, (sockaddr*)&clientAddr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int err = errno;
        switch (err)
        {
            case EAGAIN:
            case EINTR:
            case ECONNABORTED:
            case EPROTO:
            case EPERM:
            case EMFILE:
                break;
            default:
                assert(false && "accept: unexcepted error");
                break;
        }
    }
    return connfd;
}

ssize_t read(int sockfd, void* buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t write(int sockfd, const void* buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        assert(false && "close error");
    }
}

void shutdown(int sockfd, bool write)
{
    int how = write ? SHUT_WR : SHUT_RDWR;
    if (::shutdown(sockfd, how) < 0)
    {
        assert(false && "shutdown error");
    }
}

int getsockerror(int sockfd)
{
    int optval = 0;
    socklen_t optlen = socklen_t(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        int err = errno;
        return err;
    }
    return optval;
}

void setTcpNoDelay(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0)
    {
        assert(false && "set tcp no delay failed");
    }
}

void setReuseAddr(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        assert(false && "set reuse addr failed");
    }
}

void setReusePort(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        assert(false && "set reuse port failed");
    }
}

void setKeepAlive(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
    {
        assert(false && "set keepalive failed");
    }
}

}
}
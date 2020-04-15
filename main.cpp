#include <sys/epoll.h>
#include "socket.h"
#include "network.h"
#include <cstdio>
#include <errno.h>
#include "acceptor.h"

using namespace fantuan;

int main(int argc, char* argv[])
{
    Acceptor acceptor(8081);
    acceptor.listen();
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd < 0)
        return -1;
    struct epoll_event event;
    event.data.fd = acceptor.getAcceptorFd();
    event.events = EPOLLIN | EPOLLOUT;
    int s = epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptor.getAcceptorFd(), &event);
    if (s < 0)
        return -1;
    epoll_event events[10];
    while (true)
    {
        int n = epoll_wait(epollfd, events, 10, -1);
        for (int i = 0; i < n; ++i)
        {
            if (acceptor.getAcceptorFd() == events[i].data.fd)
            {
                int fd = acceptor.handleRead();
                event.data.fd = fd;
                event.events = EPOLLIN | EPOLLOUT;
                int x = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
                if (x == -1)
                    return -1;
            }
            else
            {
                size_t count;
                char buf[512];
                count = network::read(events[i].data.fd, buf, sizeof buf);
                if (count == -1)
                {
                    if (errno != EAGAIN)
                    {
                        return -1;
                    }
                }
                else if (count == 0)
                {

                }
                else printf("%s\n", buf);
            }
        }
    }
    return 0;
}
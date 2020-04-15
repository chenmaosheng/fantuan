#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char* argv[])
{
    int listenfd, connfd, sockfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) return -1;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(8081);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    if (listen(listenfd, SOMAXCONN) < 0)
        return -1;
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd < 0)
        return -1;
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLOUT;
    int s = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
    if (s < 0)
        return -1;
    epoll_event events[10];
    while (true)
    {
        int n = epoll_wait(epollfd, events, 10, -1);
        for (int i = 0; i < n; ++i)
        {
            if (listenfd == events[i].data.fd)
            {
                struct sockaddr in_addr;
                socklen_t in_len;
                int fd = accept(listenfd, &in_addr, &in_len);
                if (fd == -1)
                {
                    int err = errno;
                    if (err == EAGAIN || err == EWOULDBLOCK)
                    {

                    }
                    else
                    {
                        continue;
                    }
                }
                printf("accept\n");
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
                count = read(events[i].data.fd, buf, sizeof buf);
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"
#include <strings.h>
#include <errno.h>
#include "network.h"

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
}

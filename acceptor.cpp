#include "acceptor.h"
#include "network.h"
#include <stdio.h>
#include "connection.h"

namespace fantuan
{
Acceptor::Acceptor(uint16_t port, bool reuseport) : 
    m_AcceptSocket(network::createsocket()),
    m_Listening(false)
{
    m_AcceptSocket.setReuseAddr(true);
    m_AcceptSocket.setReusePort(reuseport);
    m_AcceptSocket.bind(port);
}

Acceptor::~Acceptor()
{

}

void Acceptor::listen()
{
    m_Listening = true;
    m_AcceptSocket.listen();
}

int Acceptor::handleRead()
{
    int connfd = m_AcceptSocket.accept();
    if (connfd >= 0)
    {
        printf("accept\n");
        newConnection(connfd);
        // TODO: 
    }
    else
    {
        // TODO: fatal error;
    }
    return connfd;
}

void Acceptor::newConnection(int sockfd)
{
    Connection* conn = new Connection(sockfd, this);
}
}
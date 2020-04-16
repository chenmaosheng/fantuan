#include "connection.h"
#include "context.h"
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include "network.h"

namespace fantuan
{
Connection::Connection(int sockfd, Acceptor* acceptor) :
    m_Socket(new Socket(sockfd)),
    m_Acceptor(acceptor)
{
    m_Context = new Context(sockfd, this);
    m_Context->setEvents(EPOLLIN | EPOLLOUT);
    bzero(&m_InputBuffer, sizeof(m_InputBuffer));
    bzero(&m_OutputBuffer, sizeof(m_OutputBuffer));
}

Connection::~Connection()
{

}

void Connection::handleRead()
{
    ssize_t count;
    count = network::read(m_Socket->getSockFd(), m_InputBuffer, sizeof(m_InputBuffer));
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
    else printf("%s\n", m_InputBuffer);
}


}
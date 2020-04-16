#include "context.h"
#include "connection.h"
#include <sys/epoll.h>

namespace fantuan
{
Context::Context(int sockfd, Connection* conn) :
    m_sockfd(sockfd),
    m_Connection(conn)
{

}

Context::~Context()
{
    
}

void Context::handleEvent()
{
    if (m_Events & EPOLLIN)
    {
        m_Connection->handleRead();
    }
    else if (m_Events & EPOLLOUT)
    {

    }
}

}
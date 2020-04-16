#include "context.h"
#include "connection.h"
#include <stdio.h>

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
        m_Connection->handleWrite();
    }
}

void Context::enableWriting()
{
    m_Events |= EPOLLOUT;
    Acceptor* acceptor = m_Connection->getAcceptor();
    acceptor->updateContext(this);
}

void Context::disableWriting()
{
    m_Events &= ~EPOLLOUT;
    Acceptor* acceptor = m_Connection->getAcceptor();
    acceptor->updateContext(this);
}

}
#include "connection.h"
#include "context.h"

namespace fantuan
{
Connection::Connection(int sockfd, Acceptor* acceptor) :
    m_Socket(new Socket(sockfd)),
    m_Acceptor(acceptor),
    m_Context(new Context(sockfd))
{
    m_Context->setEvents(EPOLLIN | EPOLLOUT);
}

Connection::~Connection()
{

}


}
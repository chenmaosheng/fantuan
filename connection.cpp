#include "connection.h"

namespace fantuan
{
Connection::Connection(int sockfd, Acceptor* acceptor) :
    m_Socket(new Socket(sockfd)),
    m_Acceptor(acceptor)
{

}

Connection::~Connection()
{

}


}
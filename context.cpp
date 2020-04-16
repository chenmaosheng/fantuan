#include "context.h"
#include "connection.h"

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


}
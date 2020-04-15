#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "socket.h"
#include "acceptor.h"

namespace fantuan
{
class Connection
{
public:
    Connection(int sockfd, Acceptor* acceptor);
    ~Connection();

private:
    Socket* m_Socket;
    Acceptor* m_Acceptor;
};
}

#endif // _H_CONNECTION
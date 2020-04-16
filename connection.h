#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "socket.h"
#include "acceptor.h"
#include "context.h"

namespace fantuan
{
class Connection
{
public:
    Connection(int sockfd, Acceptor* acceptor);
    ~Connection();

    Context* getContext() const
    {
        return m_Context;
    }

    void handleRead();

private:
    Socket* m_Socket;
    Acceptor* m_Acceptor;
    Context* m_Context;
    char m_InputBuffer[65500];
    char m_OutputBuffer[1024];
};
}

#endif // _H_CONNECTION
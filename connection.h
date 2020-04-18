#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "acceptor.h"
#include "context.h"

namespace fantuan
{
class Connection
{
public:
    Connection(int sockfd, Acceptor* acceptor, const ConnectionHandler& handler);
    ~Connection();

    int getSockfd() const
    {
        return m_sockfd;
    }
    
    Context* getContext() const
    {
        return m_Context;
    }

    Acceptor* getAcceptor() const
    {
        return m_Acceptor;
    }

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void send(const void* data, uint32_t len);
    void connectEstablished();
    void connectDestroyed();
    void setCloseHandler(const OnClose& handler)
    {
        m_CloseHandler = std::move(handler);
    }

private:
    int m_sockfd;
    Acceptor* m_Acceptor;
    Context* m_Context;
    char m_InputBuffer[65500];
    char m_OutputBuffer[1024];
    const ConnectionHandler& m_Handler;
    OnClose m_CloseHandler;
};
}

#endif // _H_CONNECTION
#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "acceptor.h"
#include "context.h"
#include "buffer.h"

namespace fantuan
{
class Connection
{
public:
    enum State { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
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
    bool Connected() const
    {
        return m_State == CONNECTED;
    }
    bool Disconnected() const
    {
        return m_State == DISCONNECTED;
    }

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void shutdown();

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
    char m_InputBuffer[4096*4];
    Buffer m_OutputBuffer;
    const ConnectionHandler& m_Handler;
    OnClose m_CloseHandler;
    State m_State;
};
}

#endif // _H_CONNECTION
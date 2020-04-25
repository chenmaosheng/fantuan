#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "acceptor.h"
#include "context.h"
#include "buffer.h"

namespace fantuan
{
class Worker;
class Connection
{
public:
    enum State { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    Connection(Worker* worker, int sockfd, const ConnectionHandler& handler, bool et);
    ~Connection();

    int getSockfd() const
    {
        return m_sockfd;
    }
    
    Context* getContext() const
    {
        return m_Context;
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
    void connectEstablished(bool et = false);
    void connectDestroyed();
    void setCloseHandler(const OnClose& handler)
    {
        m_CloseHandler = std::move(handler);
    }

private:
    int m_sockfd;
    Context* m_Context;
    char m_InputBuffer[4096];
    Buffer m_OutputBuffer;
    const ConnectionHandler& m_Handler;
    OnClose m_CloseHandler;
    State m_State;
    Worker* m_Worker;
    bool m_et;
};
}

#endif // _H_CONNECTION
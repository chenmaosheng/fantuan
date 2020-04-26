#ifndef _H_CONNECTION
#define _H_CONNECTION

#include "buffer.h"
#include "handler.h"

namespace fantuan
{
class Worker;
class Context;
class Connection
{
public:
    enum State { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    Connection(Worker* worker, int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et);
    ~Connection();

    int getSockfd() const
    {
        return m_sockfd;
    }
    bool Connected() const
    {
        return m_state == CONNECTED;
    }
    bool Disconnected() const
    {
        return m_state == DISCONNECTED;
    }
    
    void shutdown();
    void send(const void* data, uint32_t len);
    void connectEstablished();
    void connectDestroyed();
    void setRemoveConnectionHandler(const OnRemoveConnection& handler)
    {
        m_removeConnectionHandler = std::move(handler);
    }

private:
    void _handleRead(time_t time);
    void _handleWrite();
    void _handleClose();
    void _handleError();

private:
    Worker*                     m_worker;
    int                         m_sockfd;
    sockaddr_in                 m_sockaddr;
    Context*                    m_context;
    char                        m_inputBuffer[16384];
    Buffer<16384>               m_outputBuffer;
    const ConnectionHandler&    m_handler;
    OnRemoveConnection          m_removeConnectionHandler;
    State                       m_state;
    bool                        m_et;
};
}

#endif // _H_CONNECTION
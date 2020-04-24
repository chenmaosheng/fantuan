#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include "context.h"
#include <unordered_map>

namespace fantuan
{
class Worker;
class WorkerPool;
class Acceptor
{
public:
    Acceptor(uint16_t port, int numThreads=0, bool et=false);
    ~Acceptor();

    bool isListening() const
    {
        return m_Listening;
    }
    bool isEt() const
    {
        return m_et;
    }

    // accept
    void listen();
    int handleRead();
    // server
    void setHandler(const ConnectionHandler& handler)
    {
        m_Handler = std::move(handler);
    }
    void start();

private:
    // server
    void _newConnection(int sockfd);
    void _removeConnection(Connection* conn);
    void _postHandleEvent(int sockfd);

private:
    // accept
    int m_acceptfd;
    int m_idlefd;
    bool m_Listening;
    bool m_et;
    Worker* m_Worker;
    Context m_AcceptContext;
    // server
    ConnectionHandler m_Handler;
    std::unordered_map<int, Connection*> m_Connections;
    WorkerPool* m_WorkerPool;
    PostEventHandler m_PostEventHandler;
};
}

#endif // _H_ACCEPTOR
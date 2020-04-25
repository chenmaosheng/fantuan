#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include "context.h"

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

    void listen();
    int handleRead();
    // server
    void setHandler(const ConnectionHandler& handler)
    {
        m_Handler = std::move(handler);
    }
    void start();

private:
    void _newConnection(int sockfd);

private:
    // server
    Worker* m_AcceptorWorker;
    WorkerPool* m_WorkerPool;
    ConnectionHandler m_Handler;
    // accept
    int m_acceptfd;
    int m_idlefd;
    bool m_Listening;
    Worker* m_Worker;
    Context m_AcceptContext;
    bool m_et;
};
}

#endif // _H_ACCEPTOR
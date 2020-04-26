#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include "handler.h"

namespace fantuan
{
class Worker;
class WorkerPool;
class Context;
class Acceptor
{
public:
    Acceptor(uint16_t port, int numThreads=0, bool et=false);
    ~Acceptor();

    void    start();
    void    setHandler(ConnectionHandler handler)
    {
        m_handler = std::move(handler);
    }

private:
    void    _listen();
    int     _handleRead(time_t time);
    void    _addNewConnection(int sockfd, const sockaddr_in& addr);

private:
    // server
    Worker*             m_mainWorker;
    WorkerPool*         m_workerPool;
    ConnectionHandler   m_handler;
    // accept
    int                 m_acceptfd;
    int                 m_idlefd;
    Context*            m_context;
    bool                m_et;

};
}

#endif // _H_ACCEPTOR
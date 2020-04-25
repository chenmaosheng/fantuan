#ifndef _H_SERVER
#define _H_SERVER

#include "handler.h"
#include <unordered_map>

namespace fantuan
{
class WorkerPool;
class Worker;
class Acceptor;
class Connection;
class Server
{
public:
    Server(uint16_t port, int numThreads=0, bool et=false);
    ~Server();

    void setHandler(const ConnectionHandler& handler)
    {
        m_Handler = std::move(handler);
    }
    void start();

private:
    void _newConnection(int sockfd);
    void _removeConnection(Connection* conn);

private:
    ConnectionHandler m_Handler;
    std::unordered_map<int, Connection*> m_Connections;
    Worker* m_AcceptorWorker;
    Acceptor* m_Acceptor;
    WorkerPool* m_WorkerPool;
    bool m_et;
};
}

#endif // _H_SERVER
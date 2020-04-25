#include "server.h"
#include "acceptor.h"
#include "worker.h"
#include "workerpool.h"
#include "connection.h"
#include "common/utils.h"

namespace fantuan
{
Server::Server(uint16_t port, int numThreads, bool et) :
    m_et(et),
    m_AcceptorWorker(new Worker),
    m_Acceptor(new Acceptor(m_AcceptorWorker, port)),
    m_WorkerPool(new WorkerPool(m_AcceptorWorker, numThreads))
{
    m_Acceptor->SetOnNewConnection(std::bind(&Server::_newConnection, this, _1));
}

Server::~Server()
{
    for (auto& item : m_Connections)
    {
        _removeConnection(item.second);
    }
    SAFE_DELETE(m_AcceptorWorker);
    SAFE_DELETE(m_Acceptor);
    SAFE_DELETE(m_WorkerPool);
}

void Server::start()
{
    m_WorkerPool->start();
    m_Acceptor->listen();
    m_AcceptorWorker->loop();
}

void Server::_newConnection(int sockfd)
{
    Worker* worker = m_WorkerPool->getNext();
    Connection* conn = new Connection(worker, sockfd, m_Handler, m_et);
    DEBUG("sock=%d, worker=%p\n", sockfd, worker);
    m_Connections[sockfd] = conn;
    conn->setRemoveConnectionHandler([=](Connection* conn){this->_removeConnection(conn);});
    conn->connectEstablished(); // TODO: from main thread to worker thread, don't block main thread
}

void Server::_removeConnection(Connection* conn)
{
    conn->connectDestroyed();
    int sockfd = conn->getSockfd();
    m_Connections.erase(sockfd);
    SAFE_DELETE(conn);
    DEBUG("sock=%d, conn %d\n", sockfd, (int)m_Connections.size());
}

}
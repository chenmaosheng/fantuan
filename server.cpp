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
    m_PostEventHandler = [=](int sockfd){this->_postHandleEvent(sockfd);};
    m_AcceptorWorker->SetPostEventHandler(m_PostEventHandler);
}

Server::~Server()
{
    for (auto& item : m_Connections)
    {
        item.second->connectDestroyed();
    }
    SAFE_DELETE(m_AcceptorWorker);
    SAFE_DELETE(m_Acceptor);
    SAFE_DELETE(m_WorkerPool);
}

void Server::start()
{
    m_WorkerPool->start();
    const std::vector<Worker*>& workers = m_WorkerPool->getWorkers();
    for (auto worker : workers)
    {
        worker->SetPostEventHandler(m_PostEventHandler);
    }
    m_Acceptor->listen();
    m_AcceptorWorker->loop();
}

void Server::_newConnection(int sockfd)
{
    Worker* worker = m_WorkerPool->getNext();
    Connection* conn = new Connection(worker, sockfd, m_Handler, m_et);
    DEBUG("sock=%d, worker=%p\n", sockfd, worker);
    m_Connections[sockfd] = conn;
    conn->setCloseHandler([=](Connection* conn){this->_removeConnection(conn);});
    conn->connectEstablished(m_et);
}

void Server::_removeConnection(Connection* conn)
{
    if (conn)
    {
        conn->connectDestroyed();
    }
}

void Server::_postHandleEvent(int sockfd)
{
    auto mit = m_Connections.find(sockfd);
    if (mit != m_Connections.end())
    {
        Connection* conn = mit->second;
        if (conn->Disconnected())
        {
            m_Connections.erase(sockfd);
            SAFE_DELETE(conn);
            DEBUG("sock=%d, conn %d\n", sockfd, (int)m_Connections.size());
        }
    }
}

}
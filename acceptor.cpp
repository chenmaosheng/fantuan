#include "acceptor.h"
#include "network.h"
#include <stdio.h>
#include "connection.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include "common/utils.h"
#include "worker.h"
#include "workerpool.h"

namespace fantuan
{

Acceptor::Acceptor(uint16_t port, int numThreads, bool et) : 
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_Listening(false),
    m_et(et),
    m_Worker(new Worker),
    m_AcceptContext(m_Worker, m_acceptfd),
    m_WorkerPool(new WorkerPool(m_Worker, numThreads))
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, true);
    network::bind(m_acceptfd, port);
    ContextHandler handler;
    handler.m_ReadHandler = [=](){this->handleRead();};
    m_AcceptContext.setHandler(handler);
    m_PostEventHandler = [=](int sockfd){this->_postHandleEvent(sockfd);};
    m_Worker->SetPostEventHandler(m_PostEventHandler);
}

Acceptor::~Acceptor()
{
    for (auto& item : m_Connections)
    {
        item.second->connectDestroyed();
    }
    // acceptor
    m_AcceptContext.disableAll();
    m_AcceptContext.remove();
    SAFE_DELETE(m_Worker);
    SAFE_DELETE(m_WorkerPool);
    ::close(m_idlefd);
}

void Acceptor::listen()
{
    m_Listening = true;
    network::listen(m_acceptfd);
    m_AcceptContext.enableReading();
}

void Acceptor::start()
{
    m_WorkerPool->start();
    const std::vector<Worker*>& workers = m_WorkerPool->getWorkers();
    for (auto worker : workers)
    {
        worker->SetPostEventHandler(m_PostEventHandler);
    }
    listen();
    m_Worker->loop();
}

int Acceptor::handleRead()
{
    int connfd = network::accept(m_acceptfd);
    if (connfd >= 0)
    {
        _newConnection(connfd);
    }
    else
    {
        ERROR("accept failed, err=%d\n", errno);
        assert(false && "accept failed");
        // if file descriptor has used up, it'll cause accept failure and return EMFILE error
        // but it doesn't refuse this connection, still in connection queue, and still can 
        // trigger fd read event, this will cause busy loop. Use the following way can refuse
        // connection gracefully
        if (errno == EMFILE)
        {
            ::close(m_idlefd);
            m_idlefd = ::accept(m_acceptfd, NULL, NULL);
            ::close(m_idlefd);
            m_idlefd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
    return connfd;
}

void Acceptor::_newConnection(int sockfd)
{
    Worker* worker = m_WorkerPool->getNext();
    Connection* conn = new Connection(worker, sockfd, this, m_Handler);
    DEBUG("sock=%d, worker=%p\n", sockfd, worker);
    m_Connections[sockfd] = conn;
    conn->setCloseHandler([=](Connection* conn){this->_removeConnection(conn);});
    conn->connectEstablished(m_et);
}

void Acceptor::_removeConnection(Connection* conn)
{
    if (conn)
    {
        conn->connectDestroyed();
    }
}

void Acceptor::_postHandleEvent(int sockfd)
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
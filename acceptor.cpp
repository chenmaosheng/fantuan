#include "acceptor.h"
#include "network.h"
#include "connection.h"
#include "utils.h"
#include "worker.h"
#include "workerpool.h"
#include "context.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

namespace fantuan
{

Acceptor::Acceptor(uint16_t port, int numThreads, bool et) : 
    m_mainWorker(new Worker(true)),
    m_workerPool(new WorkerPool(m_mainWorker, numThreads)),
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_context(new Context(m_mainWorker, m_acceptfd)),
    m_et(et)
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, true);
    network::bind(m_acceptfd, port);
    m_context->setReadHandler([=](int64_t time){this->_handleRead(time);});
}

Acceptor::~Acceptor()
{
    m_context->disableAll();
    m_context->remove();
    SAFE_DELETE(m_context);
    ::close(m_idlefd);
    SAFE_DELETE(m_mainWorker);
    SAFE_DELETE(m_workerPool);
}

void Acceptor::start()
{
    m_workerPool->start();
    _listen();
    m_mainWorker->loop();
}

int Acceptor::_handleRead(time_t time)
{
    sockaddr_in clientAddr;
    int connfd = network::accept(m_acceptfd, clientAddr);
    if (connfd >= 0)
    {
        _addNewConnection(connfd, clientAddr);
    }
    else
    {
        ERROR("accept failed, err=%d\n", errno);
        //assert(false && "accept failed");
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

void Acceptor::_listen()
{
    network::listen(m_acceptfd);
    m_context->enableReading();
}

void Acceptor::_addNewConnection(int sockfd, const sockaddr_in& addr)
{
    Worker* worker = m_workerPool->getNext();
    worker->runFunctor(std::bind(&Worker::addConnection, worker, sockfd, addr, m_handler, m_et));
    DEBUG("sock=%d, worker=%p\n", sockfd, worker);
}

}
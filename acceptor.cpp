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
    m_AcceptorWorker(new Worker(true)),
    m_WorkerPool(new WorkerPool(m_AcceptorWorker, numThreads)),
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_Listening(false),
    m_AcceptContext(m_AcceptorWorker, m_acceptfd),
    m_et(et)
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, true);
    network::bind(m_acceptfd, port);
    ContextHandler handler;
    handler.m_ReadHandler = [=](){this->handleRead();};
    m_AcceptContext.setHandler(handler);
}

Acceptor::~Acceptor()
{
    m_AcceptContext.disableAll();
    m_AcceptContext.remove();
    ::close(m_idlefd);
    SAFE_DELETE(m_AcceptorWorker);
    SAFE_DELETE(m_WorkerPool);
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
    listen();
    m_AcceptorWorker->loop();
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

void Acceptor::_newConnection(int sockfd)
{
    Worker* worker = m_WorkerPool->getNext();
    worker->newConnection(sockfd, m_Handler, m_et);
    DEBUG("sock=%d, worker=%p\n", sockfd, worker);
}

}
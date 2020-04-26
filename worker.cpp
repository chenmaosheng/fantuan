#include "worker.h"
#include "utils.h"
#include "context.h"
#include "poller.h"
#include "connection.h"
#include "network.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <sys/eventfd.h>

struct IgnorePipe
{
IgnorePipe() {::signal(SIGPIPE, SIG_IGN);} // ignore sigpipe, must run before socket initialization
};
IgnorePipe obj;

namespace fantuan
{
Worker::Worker(bool mainWorker) : 
    m_mainWorker(mainWorker),
    m_quit(false), 
    m_thread(nullptr),
    m_poller(new Poller),
    m_wakeupFd(network::createeventfd()),
    m_wakeupContext(new Context(this, m_wakeupFd))
{
    m_wakeupContext->setReadHandler([=](int64_t time){this->_handleWakeupRead(time);});
    m_wakeupContext->enableReading();
}

Worker::~Worker()
{
    if (m_thread)
    {
        m_quit = true;
        m_thread->join();
        SAFE_DELETE(m_thread);
    }
    SAFE_DELETE(m_poller);
    m_wakeupContext->disableAll();
    m_wakeupContext->remove();
    SAFE_DELETE(m_wakeupContext);
    ::close(m_wakeupFd);
}

void Worker::loop()
{
    while (!m_quit)
    {
        m_activeContexts.clear();
        m_poller->poll(m_activeContexts);
        DEBUG("worker=%p, epoll event n=%d\n", this, (int)m_activeContexts.size());
        for (auto& context : m_activeContexts)
        {
            context->handleEvent();
        }
        if (!m_mainWorker)
        {
            _handlePendingNewConnections();
        }
    }
}

void Worker::quit()
{
    m_quit = true;
}

void Worker::runFunctor(Functor functor)
{
    if (m_mainWorker)
    {
        functor();
    }
    else
    {
        queueFunctor(functor);
    }
}

void Worker::queueFunctor(Functor functor)
{
    std::unique_lock<std::mutex> lock(m_pendingFunctorsMutex);
    m_pendingFunctors.emplace_back(std::move(functor));
    TRACE("worker=%p, queue size=%d\n", this, (int)m_pendingFunctors.size());
    if (!m_mainWorker)
    {
        _wakeup();
        DEBUG("worker=%p, wakeup\n", this);
    }
}

void Worker::updateContext(Context* context)
{
    m_poller->updateContext(context);
}

void Worker::removeContext(Context* context)
{
    m_poller->removeContext(context);
}

std::thread* Worker::startThread()
{
    m_thread = new std::thread(std::bind(&Worker::loop, this));
    return m_thread;
}

void Worker::addConnection(int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et)
{
    Connection* conn = new Connection(this, sockfd, addr, handler, et);
    m_connections[sockfd] = conn;
    conn->setRemoveConnectionHandler([=](Connection* conn){this->_removeConnection(conn);});
    conn->connectEstablished();
}

void Worker::_removeConnection(Connection* conn)
{
    conn->connectDestroyed();
    int sockfd = conn->getSockfd();
    m_connections.erase(sockfd);
    SAFE_DELETE(conn);
    DEBUG("worker=%p, sock=%d, conn %d\n", this, sockfd, (int)m_connections.size());
}

void Worker::_handlePendingNewConnections()
{
    std::vector<Functor> tmp;
    {
        std::unique_lock<std::mutex> lock(m_pendingFunctorsMutex);
        tmp.swap(m_pendingFunctors); // swap just in case functor call queue again to add new functor
        TRACE("worker=%p, tmp size=%d\n", this, (int)tmp.size());
    }
    for (auto& functor : tmp)
    {
        functor();
    }
}

void Worker::_handleWakeupRead(time_t time)
{
    uint64_t tmp = 1;
    ssize_t n = network::read(m_wakeupFd, &tmp, sizeof(tmp));
    if (n != sizeof(tmp))
    {
        ERROR("_handleWakeupRead failed, n=%d\n", (int)n);
    }
}

void Worker::_wakeup()
{
    uint64_t tmp = 1;
    ssize_t n = network::write(m_wakeupFd, &tmp, sizeof(tmp));
    if (n != sizeof(tmp))
    {
        ERROR("_wakeup failed, n=%d\n", (int)n);
    }
}

}
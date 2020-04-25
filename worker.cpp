#include "worker.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include "common/utils.h"
#include "context.h"
#include "poller.h"
#include "connection.h"
#include "network.h"
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
    m_handlePendingNewConnections(false),
    m_Quit(false), 
    m_Thread(nullptr),
    m_Poller(new Poller)
{
    m_WakeupFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (m_WakeupFd < 0)
    {
        ERROR("create eventfd failed, err=%d\n", errno);
        abort();
    }
    m_WakeupContext = new Context(this, m_WakeupFd);
    ContextHandler contextHandler;
    contextHandler.m_ReadHandler = [=](){this->_handleWakeupRead();};
    m_WakeupContext->setHandler(contextHandler);
    m_WakeupContext->enableReading();
}

Worker::~Worker()
{
    if (m_Thread)
    {
        m_Quit = true;
        m_Thread->join();
        SAFE_DELETE(m_Thread);
    }
    SAFE_DELETE(m_Poller);
    m_WakeupContext->disableAll();
    m_WakeupContext->remove();
    SAFE_DELETE(m_WakeupContext);
    ::close(m_WakeupFd);
}

void Worker::loop()
{
    while (!m_Quit)
    {
        m_ActiveContexts.clear();
        m_Poller->poll(m_ActiveContexts);
        DEBUG("worker=%p, epoll event n=%d\n", this, (int)m_ActiveContexts.size());
        for (auto& context : m_ActiveContexts)
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
    m_Quit = true;
}

void Worker::newConnection(int sockfd, const ConnectionHandler& handler, bool et)
{
    if (m_mainWorker)
    {
        _newConnection(sockfd, handler, et);
    }
    else
    {
        queueNewConnection(sockfd, handler, et);
    }
}

void Worker::queueNewConnection(int sockfd, const ConnectionHandler& handler, bool et)
{
    std::unique_lock<std::mutex> lock(m_PendingNewConnectionsMutex);
    m_PendingNewConnections.emplace_back(NewConnectionParams(sockfd, handler, et));
    DEBUG("worker=%p, queue size=%d\n", this, (int)m_PendingNewConnections.size());
    if (!m_mainWorker || m_handlePendingNewConnections)
    {
        _wakeup();
        DEBUG("worker=%p, wakeup\n", this);
    }
}

void Worker::updateContext(Context* context)
{
    m_Poller->updateContext(context);
}

void Worker::removeContext(Context* context)
{
    m_Poller->removeContext(context);
}

void Worker::startThread()
{
    m_Thread = new std::thread(std::bind(&Worker::loop, this));
}

void Worker::_newConnection(int sockfd, const ConnectionHandler& handler, bool et)
{
    Connection* conn = new Connection(this, sockfd, handler, et);
    m_Connections[sockfd] = conn;
    conn->setRemoveConnectionHandler([=](Connection* conn){this->_removeConnection(conn);});
    conn->connectEstablished();
}

void Worker::_removeConnection(Connection* conn)
{
    conn->connectDestroyed();
    int sockfd = conn->getSockfd();
    m_Connections.erase(sockfd);
    SAFE_DELETE(conn);
    DEBUG("worker=%p, sock=%d, conn %d\n", this, sockfd, (int)m_Connections.size());
}

void Worker::_handlePendingNewConnections()
{
    m_handlePendingNewConnections = true;
    std::vector<NewConnectionParams> tmp;
    {
        std::unique_lock<std::mutex> lock(m_PendingNewConnectionsMutex);
        tmp.swap(m_PendingNewConnections); // swap just in case functor call queue again to add new functor
        DEBUG("worker=%p, tmp size=%d\n", this, (int)tmp.size());
    }
    for (auto& params : tmp)
    {
        _newConnection(params.m_sockfd, params.m_Handler, params.m_et);
    }
    m_handlePendingNewConnections = false;
}

void Worker::_handleWakeupRead()
{
    uint64_t tmp = 1;
    ssize_t n = network::read(m_WakeupFd, &tmp, sizeof(tmp));
    if (n != sizeof(tmp))
    {
        ERROR("_handleWakeupRead failed, n=%d\n", (int)n);
    }
}

void Worker::_wakeup()
{
    uint64_t tmp = 1;
    ssize_t n = network::write(m_WakeupFd, &tmp, sizeof(tmp));
    if (n != sizeof(tmp))
    {
        ERROR("_wakeup failed, n=%d\n", (int)n);
    }
}

}
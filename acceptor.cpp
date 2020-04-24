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

namespace fantuan
{

Acceptor::Acceptor(uint16_t port, bool et) : 
    m_acceptfd(network::createsocket()),
    m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_Listening(false),
    m_et(et),
    m_Worker(new Worker),
    m_AcceptContext(m_Worker, m_acceptfd)
{
    network::setTcpNoDelay(m_acceptfd, true);
    network::setReusePort(m_acceptfd, true);
    network::bind(m_acceptfd, port);
    ContextHandler handler;
    handler.m_ReadHandler = [=](){this->handleRead();};
    m_AcceptContext.setHandler(handler);

    m_Handler.m_OnConnection = [=](Connection*){};
    m_Handler.m_OnDisconnected = [=](Connection*){};
    m_Handler.m_OnData = [=](Connection*, uint16_t, char*){};
    m_Worker->SetPostEventHandler([=](int sockfd){this->_postHandleEvent(sockfd);});
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
        PRINTF("accept failed, err=%d\n", errno);
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
     Connection* conn = new Connection(m_Worker, sockfd, this, m_Handler);
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
            PRINTF("conn %d\n", (int)m_Connections.size());
        }
    }
}

}
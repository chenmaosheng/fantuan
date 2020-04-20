#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include <vector>
#include <sys/epoll.h>
#include "context.h"
#include <unordered_map>

namespace fantuan
{
class Acceptor
{
public:
    Acceptor(uint16_t port);
    ~Acceptor();

    bool isListening() const
    {
        return m_Listening;
    }

    // accept
    void listen();
    int handleRead();
    // epoll
    void poll();
    void updateContext(Context* context);
    void removeContext(Context* context);
    // server
    void setHandler(const ConnectionHandler& handler)
    {
        m_Handler = std::move(handler);
    }

private:
    void _updateContext(int operation, Context* context);
    // server
    void _newConnection(int sockfd);
    void _removeConnection(Connection* conn);

private:
    // accept
    int m_acceptfd;
    int m_idlefd;
    bool m_Listening;
    Context m_AcceptContext;
    // epoll
    const static int m_InitEventListSize = 16;
    int m_epollfd;
    std::vector<epoll_event> m_EventList;
    // server
    ConnectionHandler m_Handler;
    std::unordered_map<int, Connection*> m_Connections;
};
}

#endif // _H_ACCEPTOR
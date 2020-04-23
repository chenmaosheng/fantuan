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
    Acceptor(uint16_t port, bool et = false);
    ~Acceptor();

    bool isListening() const
    {
        return m_Listening;
    }
    bool isEt() const
    {
        return m_et;
    }

    // accept
    void listen();
    int handleRead();
    // epoll
    void poll(int timeout=10000); // 10000ms
    void updateContext(Context* context);
    void removeContext(Context* context);
    // server
    void setHandler(const ConnectionHandler& handler)
    {
        m_Handler = std::move(handler);
    }

private:
    // epoll
    void _updateContext(int operation, Context* context);
    // server
    void _newConnection(int sockfd);
    void _removeConnection(Connection* conn);
    void _postHandleEvent(int sockfd);

private:
    // accept
    int m_acceptfd;
    int m_idlefd;
    bool m_Listening;
    bool m_et;
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
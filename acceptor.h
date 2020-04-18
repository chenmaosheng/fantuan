#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include <vector>
#include <sys/epoll.h>
#include "context.h"

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

    int getAcceptorFd()
    {
        return m_acceptfd;
    }

    void listen();
    int handleRead();
    void poll();
    void updateContext(Context* context);
    void removeContext(Context* context);

private:
    void _updateContext(int operation, Context* context);
    // server
    void _newConnection(int sockfd);

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
    epoll_event	m_AcceptEvent;
    // server
};
}

#endif // _H_ACCEPTOR
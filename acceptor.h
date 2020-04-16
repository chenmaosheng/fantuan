#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include "socket.h"
#include <stdint.h>
#include <vector>
#include <sys/epoll.h>
#include "context.h"

namespace fantuan
{
class Acceptor
{
public:
    Acceptor(uint16_t port, bool reuseport=false);
    ~Acceptor();

    bool isListening() const
    {
        return m_Listening;
    }

    int getAcceptorFd()
    {
        return m_AcceptSocket.getSockFd();
    }

    void listen();
    int handleRead();
    void poll();

private:
    void _updateContext(int operation, Context* context);

private:
    Socket m_AcceptSocket;
    bool m_Listening;
    int m_epollfd;
    std::vector<epoll_event> m_EventList;
    epoll_event	m_AcceptEvent;
    Context m_AcceptContext;
};
}

#endif // _H_ACCEPTOR
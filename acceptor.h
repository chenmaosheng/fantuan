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

private:
    void _updateContext(int operation, Context* context);

private:
    int m_acceptfd;
    bool m_Listening;
    int m_epollfd;
    std::vector<epoll_event> m_EventList;
    epoll_event	m_AcceptEvent;
    Context m_AcceptContext;
};
}

#endif // _H_ACCEPTOR
#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include "socket.h"
#include <stdint.h>

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

private:
    Socket m_AcceptSocket;
    bool m_Listening;
};
}

#endif // _H_ACCEPTOR
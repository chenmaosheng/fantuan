#ifndef _H_ACCEPTOR
#define _H_ACCEPTOR

#include <stdint.h>
#include "context.h"

namespace fantuan
{
class Worker;
class Acceptor
{
public:
    Acceptor(Worker* worker, uint16_t port);
    ~Acceptor();

    bool isListening() const
    {
        return m_Listening;
    }

    void listen();
    int handleRead();
    void SetOnNewConnection(const OnNewConnection& handler)
    {
        m_OnNewConnection = std::move(handler);
    }

private:
    // accept
    int m_acceptfd;
    int m_idlefd;
    bool m_Listening;
    Worker* m_Worker;
    Context m_AcceptContext;
    OnNewConnection m_OnNewConnection;
};
}

#endif // _H_ACCEPTOR
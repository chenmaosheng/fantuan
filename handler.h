#ifndef _H_HANDLER
#define _H_HANDLER

#include <functional>
#include <netinet/in.h>

namespace fantuan
{
class Connection;
using OnConnection = std::function<void (Connection*)>;
using OnDisconnected = std::function<void (Connection*)>;
using OnData = std::function<void (Connection*, uint16_t, char*)>;
using OnSendData = std::function<void (Connection*)>;
struct ConnectionHandler
{
    OnConnection    m_onConnection;
    OnDisconnected  m_onDisconnected;
    OnData          m_onData;
    OnSendData      m_sendData;
};
struct NewConnectionParam
{
    int m_sockfd;
    sockaddr_in m_sockaddr;
    ConnectionHandler m_handler;
    bool m_et;
    NewConnectionParam(int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et):
        m_sockfd(sockfd), m_sockaddr(addr), m_handler(handler), m_et(et){}
};

using OnRemoveConnection = std::function<void (Connection*)>;
using OnEvent = std::function<void()>;
using OnReadEvent = std::function<void (time_t)>;
}

#endif // _H_HANDLER
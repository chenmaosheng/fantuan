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

using OnRemoveConnection = std::function<void (Connection*)>;
using OnEvent = std::function<void()>;
using OnReadEvent = std::function<void (time_t)>;
using Functor = std::function<void()>;
}

#endif // _H_HANDLER
#ifndef _H_HANDLER
#define _H_HANDLER

#include <functional>

namespace fantuan
{
class Connection;
using OnConnection = std::function<void (Connection*)>;
using OnDisconnected = std::function<void (Connection*)>;
using OnData = std::function<void (Connection*, uint16_t, char*)>;

struct ConnectionHandler
{
    OnConnection m_OnConnection;
    OnDisconnected m_OnDisconnected;
    OnData m_OnData;
};

using OnRemoveConnection = std::function<void (Connection*)>;
using std::placeholders::_1;
using OnNewConnection = std::function<void (int)>;
class Context;

using EventHandler = std::function<void()>;
struct ContextHandler
{
    EventHandler m_ReadHandler;
    EventHandler m_WriteHandler;
    EventHandler m_CloseHandler;
    EventHandler m_ErrorHandler;
};
}

#endif // _H_HANDLER
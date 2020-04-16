#ifndef _H_HANDLER
#define _H_HANDLER

#include <functional>

namespace fantuan
{
class Connection;
using OnConnection = std::function<void (const Connection&)>;
using OnDisconnected = std::function<void (const Connection&)>;
using OnData = std::function<void (const Connection&, uint16_t, char*)>;

struct ConnectionHandler
{
    OnConnection m_OnConnection;
    OnDisconnected m_OnDisconnected;
    OnData m_OnData;
};
}

#endif // _H_HANDLER
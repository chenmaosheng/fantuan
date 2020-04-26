#ifndef _H_WORKER
#define _H_WORKER

#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "handler.h"

namespace fantuan
{
class Context;
class Poller;
class Connection;
class Worker
{
public:
    Worker(bool mainWorker=false);
    ~Worker();

    void            loop();
    void            quit();
    void            addConnection(int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et);
    void            queueConnection(int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et);
    
    void            updateContext(Context* context);
    void            removeContext(Context* context);

    std::thread*    startThread();

private:
    void            _addConnection(int sockfd, const sockaddr_in& addr, const ConnectionHandler& handler, bool et);
    void            _removeConnection(Connection* conn);
    void            _handlePendingNewConnections();
    void            _handleWakeupRead(time_t time);
    void            _wakeup();
    
private:
    bool                                    m_mainWorker;
    std::atomic_bool                        m_quit;
    std::thread*                            m_thread;
    Poller*                                 m_poller;
    std::vector<Context*>                   m_activeContexts;
    std::mutex                              m_pendingNewConnectionsMutex;
    std::vector<NewConnectionParam>         m_pendingNewConnections;
    std::unordered_map<int, Connection*>    m_connections;
    int                                     m_wakeupFd;
    Context*                                m_wakeupContext;
};
}

#endif // _H_WORKER
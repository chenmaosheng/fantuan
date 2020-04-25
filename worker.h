#ifndef _H_WORKER
#define _H_WORKER

#include <vector>
#include <functional>
#include <atomic>
#include <sys/epoll.h>
#include <thread>
#include "handler.h"
#include <mutex>
#include <unordered_map>

namespace fantuan
{
class Context;
class Poller;
class Connection;
class Worker
{
public:
    struct NewConnectionParams
    {
        int m_sockfd;
        ConnectionHandler m_Handler;
        bool m_et;
        NewConnectionParams(int sockfd, const ConnectionHandler& handler, bool et):
            m_sockfd(sockfd), m_Handler(handler), m_et(et){}
    };
    Worker(bool mainWorker=false);
    ~Worker();

    void loop();
    void quit();
    void newConnection(int sockfd, const ConnectionHandler& handler, bool et);
    void queueNewConnection(int sockfd, const ConnectionHandler& handler, bool et);
    
    void updateContext(Context* context);
    void removeContext(Context* context);

    void startThread();

private:
    void _newConnection(int sockfd, const ConnectionHandler& handler, bool et);
    void _removeConnection(Connection* conn);
    void _handlePendingNewConnections();
    void _handleWakeupRead();
    void _wakeup();
    
private:
    bool m_mainWorker;
    bool m_handlePendingNewConnections;
    std::atomic_bool m_Quit;
    std::thread* m_Thread;
    Poller* m_Poller;
    std::vector<Context*> m_ActiveContexts;
    std::mutex m_PendingNewConnectionsMutex;
    std::vector<NewConnectionParams> m_PendingNewConnections;
    std::unordered_map<int, Connection*> m_Connections;
    int m_WakeupFd;
    Context* m_WakeupContext;
};
}

#endif // _H_WORKER
#ifndef _H_WORKERPOOL
#define _H_WORKERPOOL

#include <vector>
#include <sched.h>

namespace fantuan
{
class Worker;
class WorkerPool
{
public:
    WorkerPool(Worker* mainWorker, int numThreads);
    ~WorkerPool();
    
    void    start();
    Worker* getNext();

private:
    int                     m_numThreads;
    int                     m_nextThreadIndex;
    std::vector<Worker*>    m_workers;
    Worker*                 m_mainWorker;
    cpu_set_t               m_cpuAffinityMask[128];
};
}

#endif // _H_WORKERPOOL
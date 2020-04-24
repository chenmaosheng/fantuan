#ifndef _H_WORKERPOOL
#define _H_WORKERPOOL

#include <vector>

namespace fantuan
{
class Worker;
class WorkerPool
{
public:
    WorkerPool(Worker* mainWorker, int numThreads);
    ~WorkerPool();
    
    const std::vector<Worker*>& getWorkers() const { return m_Workers; }
    void start();
    Worker* getNext();

private:
    int m_numThreads;
    int m_nextThreadIndex;
    std::vector<Worker*> m_Workers;
    Worker* m_mainWorker;
};
}

#endif // _H_WORKERPOOL
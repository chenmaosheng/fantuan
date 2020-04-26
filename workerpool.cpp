#include "workerpool.h"
#include "worker.h"
#include "utils.h"
#include <pthread.h>

namespace fantuan
{
WorkerPool::WorkerPool(Worker* mainWorker, int numThreads) :
    m_numThreads(numThreads),
    m_nextThreadIndex(0),
    m_mainWorker(mainWorker)
{
    CPU_ZERO(&m_cpuAffinityMask[0]);
    CPU_SET(0, &m_cpuAffinityMask[0]);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &m_cpuAffinityMask[0]);
}

WorkerPool::~WorkerPool()
{
    for (int i = 0; i < m_numThreads; ++i)
    {
        SAFE_DELETE(m_workers[i]);
    }
    m_workers.clear();
}

void WorkerPool::start()
{
    for (int i = 1; i <= m_numThreads; ++i)
    {
        Worker* worker = new Worker;
        CPU_ZERO(&m_cpuAffinityMask[i]);
        CPU_SET(i, &m_cpuAffinityMask[i]);
        std::thread* thread = worker->startThread();
        pthread_setaffinity_np(thread->native_handle(), sizeof(cpu_set_t), &m_cpuAffinityMask[i]);
        m_workers.push_back(worker);
    }
}

Worker* WorkerPool::getNext()
{
    if (m_workers.empty()) return m_mainWorker;
    Worker* worker = m_workers[m_nextThreadIndex];
    m_nextThreadIndex++;
    if (m_nextThreadIndex == m_numThreads)
    {
        m_nextThreadIndex = 0;
    }
    return worker;
}

}
#include "workerpool.h"
#include "worker.h"
#include "common/utils.h"

namespace fantuan
{
WorkerPool::WorkerPool(Worker* mainWorker, int numThreads) :
    m_numThreads(numThreads),
    m_nextThreadIndex(0),
    m_mainWorker(mainWorker)
{

}

WorkerPool::~WorkerPool()
{
    for (int i = 0; i < m_numThreads; ++i)
    {
        SAFE_DELETE(m_Workers[i]);
    }
    m_Workers.clear();
}

void WorkerPool::start()
{
    for (int i = 0; i < m_numThreads; ++i)
    {
        Worker* worker = new Worker;
        worker->startThread();
        m_Workers.push_back(worker);
    }
}

Worker* WorkerPool::getNext()
{
    if (m_Workers.empty()) return m_mainWorker;
    Worker* worker = m_Workers[m_nextThreadIndex];
    m_nextThreadIndex++;
    if (m_nextThreadIndex == m_numThreads)
    {
        m_nextThreadIndex = 0;
    }
    return worker;
}

}
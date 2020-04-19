#ifndef _H_LOG
#define _H_LOG

#include <mutex>
#include "buffer.h"
#include <fcntl.h>
#include <condition_variable>
#include <thread>

namespace fantuan
{
class Log
{
public:
    enum { BUFFER_MAX_SIZE = 65535, };
    static Log& Instance()
    {
        static Log instance;
        return instance;
    }
    Log();
    ~Log();

    void init();
    void destroy();
    void push(const char* format, ...);
    static void _output(void* pParam);
    void tick();
    void output(char* buffer, int count);

private:
    Buffer m_Buffer;
    std::mutex m_Mutex;
    std::condition_variable	m_OutputEvent;
    std::thread* m_Thread;
    int m_File;
};

#define LOG_DBG(Expression, ...)										\
	fantuan::Log::Instance().push("[LINE:%-4u] " Expression, __LINE__, ##__VA_ARGS__);
}

#endif // _H_LOG
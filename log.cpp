#include "log.h"
#include <strings.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

namespace fantuan
{
Log::Log()
{
}    

Log::~Log()
{

}

void Log::init()
{
    m_Thread = new std::thread(&Log::_output, this);
    char name[32] = {0};
    snprintf(name, 32, "%ld.log", (long int)time(NULL));
    m_File = open(name, O_RDWR | O_CREAT);
}

void Log::destroy()
{
    m_Thread->join();
}

void Log::push(const char* format, ...)
{
    char buffer[BUFFER_MAX_SIZE] = {0}, realBuffer[BUFFER_MAX_SIZE] = {0};
	va_list Args;

	va_start(Args, format);
	int iLength = vsnprintf(buffer, BUFFER_MAX_SIZE, format, Args) + 1;
	va_end(Args);
    time_t now = time(NULL);
	tm* currentTime = localtime(&now);
	snprintf(realBuffer, BUFFER_MAX_SIZE, "[%02d.%02d.%02d %02d:%02d:%02d]%s", 
		currentTime->tm_year+1900, currentTime->tm_mon+1, currentTime->tm_mday, currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec,
		buffer);
    std::unique_lock<std::mutex> lock(m_Mutex);
	m_OutputEvent.notify_one();
	return m_Buffer.append(realBuffer, (int)strlen(realBuffer));
}

void Log::_output(void* pParam)
{
    Log* log = (Log*)pParam;
    while (true)
    {
        log->tick();
    }
}

void Log::output(char* buffer, int count)
{
    ::write(m_File, buffer, count);
}

void Log::tick()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
	m_OutputEvent.wait(lock);
	int size = m_Buffer.pendingBytes();
    char strBuffer[BUFFER_MAX_SIZE] = {0};
    memcpy(strBuffer, m_Buffer.peek(), size);
    m_Buffer.retrieve(size);
    output(strBuffer, size);
}

}
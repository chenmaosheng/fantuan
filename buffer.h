#ifndef _H_BUFFER
#define _H_BUFFER

#include <strings.h>
#include <string.h>
#include <assert.h>

namespace fantuan
{
struct Buffer
{
    Buffer() : sentIndex(0), pendingIndex(0)
    {
        bzero(buffer, sizeof(buffer));
    }

    size_t pendingBytes() const
    {
        return pendingIndex - sentIndex;
    }

    size_t availBytes() const
    {
        return maxSize - pendingIndex;
    }

    const char* peek() const
    {
        return buffer + sentIndex;
    }

    void append(const char* data, size_t len)
    {
        if (len > maxSize - pendingBytes())
        {
            assert(false && "overflow");
        }
        else
        {
            if (len > availBytes())
            {
                memmove(buffer, buffer+sentIndex, maxSize - sentIndex);
                pendingIndex -= sentIndex;
                sentIndex = 0;
            }
            memcpy(buffer+pendingIndex, data, len);
            pendingIndex += len;
        }
    }

    void retrieve(size_t len)
    {
        assert(len <= pendingBytes());
        if (len <= pendingBytes())
        {
            sentIndex += len;
        }
    }

    const static size_t maxSize = 16;
    char buffer[maxSize];
    size_t sentIndex;
    size_t pendingIndex;
};
}

#endif // _H_BUFFER
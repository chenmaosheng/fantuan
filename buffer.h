#ifndef _H_BUFFER
#define _H_BUFFER

#include <strings.h>
#include <string.h>
#include <assert.h>

namespace fantuan
{
template<size_t maxSize = 16*1024>
struct Buffer
{
    Buffer() : m_sentIndex(0), m_pendingIndex(0)
    {
        bzero(m_buffer, sizeof(m_buffer));
    }

    size_t pendingBytes() const
    {
        return m_pendingIndex - m_sentIndex;
    }

    size_t availBytes() const
    {
        return maxSize - m_pendingIndex;
    }

    const char* peek() const
    {
        return m_buffer + m_sentIndex;
    }

    size_t getSentIndex() const
    {
        return m_sentIndex;
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
                memmove(m_buffer, m_buffer+m_sentIndex, maxSize - m_sentIndex);
                m_pendingIndex -= m_sentIndex;
                m_sentIndex = 0;
            }
            memcpy(m_buffer+m_pendingIndex, data, len);
            m_pendingIndex += len;
        }
    }

    void retrieve(size_t len)
    {
        assert(len <= pendingBytes());
        if (len <= pendingBytes())
        {
            m_sentIndex += len;
        }
    }

    char    m_buffer[maxSize];
    size_t  m_sentIndex;
    size_t  m_pendingIndex;
};
}

#endif // _H_BUFFER
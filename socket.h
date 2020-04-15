#ifndef _H_SOCKET
#define _H_SOCKET

namespace fantuan
{

class Socket
{
public:
    explicit Socket(int sockfd)
        : m_sockfd(sockfd)
    {
        
    }

    ~Socket();
    //Socket(Socket&&);
    //Socket& operator=(Socket&&);

    int getSockFd() const
    {
        return m_sockfd;
    }

    void bind(uint16_t port);
    void listen();
    int accept();
    void shutdownWR();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
private:
    const int m_sockfd;
};

}

#endif // _H_SOCKET
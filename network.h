#ifndef _H_NETWORK
#define _H_NETWORK

#include <netinet/in.h>

namespace fantuan
{
namespace network
{
int     createsocket();
int     createepoll();
int     createeventfd();
void    bind(int sockfd, uint16_t port);
void    listen(int sockfd);
int     accept(int sockfd, sockaddr_in& clientAddr);
ssize_t read(int sockfd, void* buf, size_t count);
ssize_t write(int sockfd, const void* buf, size_t count);
void    close(int sockfd);
void    shutdown(int sockfd, bool wr=true);
int     getsockerror(int sockfd);
void    setTcpNoDelay(int sockfd, bool on);
void    setReuseAddr(int sockfd, bool on);
void    setReusePort(int sockfd, bool on);
void    setKeepAlive(int sockfd, bool on);
}
}

#endif // _H_NETWORK
#ifndef _H_NETWORK
#define _H_NETWORK

#include <arpa/inet.h>

namespace fantuan
{
namespace network
{
int createsocket();
void bind(int sockfd, uint16_t port);
void listen(int sockfd);
int accept(int sockfd, sockaddr* addr);
ssize_t read(int sockfd, void* buf, size_t count);
ssize_t readv(int sockfd, const iovec* iov, int count);
ssize_t write(int sockfd, const void* buf, size_t count);
void close(int sockfd);
void shutdownWR(int sockfd);
int getsockerror(int sockfd);
}
}

#endif // _H_NETWORK
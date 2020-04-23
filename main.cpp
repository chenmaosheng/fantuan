#include <sys/epoll.h>
#include "network.h"
#include <cstdio>
#include <errno.h>
#include "acceptor.h"

using namespace fantuan;

int main(int argc, char* argv[])
{
    Acceptor acceptor(8081, true);
    acceptor.listen();
    while (true)
    {
        acceptor.poll();
    }
    return 0;
}
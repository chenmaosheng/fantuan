#include <sys/epoll.h>
#include "network.h"
#include <cstdio>
#include <errno.h>
#include "acceptor.h"
#include "log.h"

using namespace fantuan;

int main(int argc, char* argv[])
{
    Log::Instance().init();
    Acceptor acceptor(8081);
    acceptor.listen();
    while (true)
    {
        acceptor.poll();
    }
    return 0;
}
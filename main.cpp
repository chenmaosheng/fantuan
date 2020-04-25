#include "server.h"

using namespace fantuan;

int main(int argc, char* argv[])
{
    int numThread = 0, isEt = 0;
    if (argc > 1)
        numThread = atoi(argv[1]);
    if (argc > 2)
        isEt = atoi(argv[2]);
    Server server(8081, numThread, isEt==1 ? true : false);
    server.start();
    return 0;
}
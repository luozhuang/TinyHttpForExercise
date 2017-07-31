#include "myserver.h"
#include "memmap.h"

int main(int argc, char *argv[])
{

    HttpServer server;
    server.Start();
    return 0;
}

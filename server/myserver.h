#ifndef __MYSERVER_H__
#define __MYSERVER_H__
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
using namespace std;
#define DEFU_SERPORT 30023
#define DEFU_BACKLOG 5
#define BUFSIZE 1024

#define ERROR_EXIT(x) \
    do { \
        perror(x); \
        exit(EXIT_FAILURE); \
    } while(0) \

class HttpServer
{
public:
    HttpServer()
    {

    }
    ~HttpServer()
    {

    }
    void Start(int port = DEFU_SERPORT);
private:
    int m_socketfd;
    char m_buffer[BUFSIZE];
};
#endif

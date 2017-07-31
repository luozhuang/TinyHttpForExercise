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
#include "global.h"
using namespace std;

#define DEF_SERPORT 30023
#define DEF_BACKLOG 5
#define BUFSIZE 1024

class HttpServer
{
public:
    HttpServer()
    {

    }
    ~HttpServer()
    {

    }

    void SignalHandler(int sig);
    void RegSignaler(int sig, void(*handler)(int), bool restart = true);
    void SetupSignaler();

    void Start(int port = DEF_SERPORT);
private:
    int m_lisentfd;
    char m_buffer[BUFSIZE];
    int m_sigfd[2];
};
#endif

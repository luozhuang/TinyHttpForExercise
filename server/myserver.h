#ifndef __MYSERVER_H__
#define __MYSERVER_H__
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
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

    //three way for echo implementing
    void Echo_Select();
    void Echo_Poll();
    void Echo_Epoll();
    void Echo(int client);
    void RemoveValue(int& index, vector<int>& client);
private:
    int m_listenfd;
    char m_buffer[BUFSIZE];

    //select for echo implement
    //fd_set m_readfds;
    struct SelectData
    {
        vector<int> client_fds;
        int maxfd_index;
        fd_set  readfds, readyfds;
    };

    SelectData m_fdsetdata;
};
#endif

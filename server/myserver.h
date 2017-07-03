#ifndef __MYSERVER_H__
#define __MYSERVER_H__
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
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

enum MULSTYLE
{
    MULTIP_SELECT,
    MULTIP_POLL,
    MULTIP_EPOLL,
    MULTIP_MAXS
};

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

    void Echo(int client, int flag);
    void RemoveValue(int& index, vector<int>& client);
    void FdCloseProcess(int fd, int flag);
private:
    int m_listenfd;
    char m_buffer[BUFSIZE];

    //echo implement in select
    typedef struct SelectData
    {
        vector<int> client_fds;
        int maxfd_index;
        fd_set  readfds, readyfds;
    } SelectData;
    SelectData m_fdsetdata;

    //echo implement for poll
    vector<pollfd> event_queue;
};
#endif

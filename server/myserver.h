#ifndef __MYSERVER_H__
#define __MYSERVER_H__

#include <vector>
#include <algorithm>
#define DEF_SERPORT 30023
#define DEF_BACKLOG 5
#define BUFSIZE 1024
#define MAX_EVENTS 100
#define USER_LIMIT 1000

class Memorymap;

class HttpServer
{
public:
    HttpServer()
        :m_curclitcount(0),
         m_memmap(nullptr),
         m_childpid(0)
    {

    }
    ~HttpServer()
    {
        if (m_memmap) {
            delete m_memmap;
        }
    }

    void SetupSignaler();
    void Start(int port = DEF_SERPORT);
    void EventForEpoll();


private:
    static void SignalHandler(int sig);
    void RegSignaler(int sig, void(*handler)(int), bool restart = true);
    void AddfdToEpoll(int, int);
    int SetfdNonBlocking(int fd);
    void SubProcess(int fd);
    void RmSubPro(int pid);
    void RmfdFromEpoll(int epoll, int fd);

private:
    int m_listenfd;
    char m_buffer[BUFSIZE];

    //transmit received signals to main process for listening
    static int m_sigfd[2];

    int m_epollfd;
    int m_curclitcount;

    Memorymap* m_memmap;

    std::vector<int> m_childpid;
};
#endif

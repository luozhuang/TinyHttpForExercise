#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <sys/stat.h>
#include <cstdio>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include "myserver.h"

#include "global.h"
#include "memmap.h"
using namespace std;

int HttpServer::m_sigfd[2];
void HttpServer::Start(int port)
{

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1) {
        ERROR_HANDLE("create socket error!");
    }

    int reuse = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in server_addr, client_addr;
    socklen_t clientaddr_len;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEF_SERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenfd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) == -1) {
            ERROR_HANDLE("bind addr failure!");
    }

    if (listen(m_listenfd, DEF_BACKLOG) == -1) {
            ERROR_HANDLE("server listening failure!");
    }

    cout<<"server listing..."<<endl;

    //create new memory map
    m_memmap = new Memorymap();
    m_memmap->MapMemory(USER_LIMIT * BUFSIZE, O_CREAT|O_RDWR, 0666);


    EventForEpoll();

    /*
    int clientfd;
    if ((clientfd = accept(m_listenfd, (struct sockaddr*)(&client_addr), &clientaddr_len)) < 0) {
        ERROR_HANDLE("connect failure!");
    }

    cout<<"connect to client success!"<<endl;
    int ret = -1;
    while ((ret = read(clientfd, m_buffer, BUFSIZE))) {
        if (ret != -1) {
            if (ret == 0) {
                cout<<"client disconnect!"<<endl;
                break;
            } else {
                cout<<m_buffer;
                ret = write(clientfd, m_buffer, BUFSIZE);
                memset(m_buffer, 0 , BUFSIZE);
                if (ret == -1) {
                    ERROR_HANDLE("write data section to client error!");
                }
            }

        }
        else {
            ERROR_HANDLE("read data section from client error!");
        }
    }

    close(clientfd);
    cout<<"Client disconnect!"<<endl;
    */
}

void HttpServer::EventForEpoll()
{
    m_epollfd = epoll_create(10);
    if (m_epollfd < 0) {
        ERROR_HANDLE("create epoll fd error!");
    }

    //receive signal and listening
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigfd) == -1) {
        ERROR_HANDLE("create socket pipe error!");
    }

    SetfdNonBlocking(m_sigfd[1]);
    AddfdToEpoll(m_epollfd, m_listenfd);
    AddfdToEpoll(m_epollfd, m_sigfd[0]);

    struct epoll_event events[MAX_EVENTS];

    bool server_stop = false, terminal = false;

    while (!server_stop) {
        int ready_eventnum = epoll_wait(m_epollfd, events, MAX_EVENTS, -1);

        if (ready_eventnum < 0 && errno != EINTR) {
            ERROR_HANDLE("epoll wait event error!");
        }

        for (int i = 0; i < ready_eventnum; ++i) {
            int currentfd = events[i].data.fd;
            if (currentfd == m_listenfd && events[i].events == EPOLLIN) {

                struct sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int clientfd = accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_addrlen);
                if (clientfd < 0) {
                    cout<<"create new connection failure!";
                    continue;
                }

                if (m_curclitcount >= USER_LIMIT) {
                    char* msg_userlimit = "user count exceed limit!  please wait!!!";
                    cout<<msg_userlimit<<endl;
                    if (send(clientfd, msg_userlimit, strlen(msg_userlimit), 0) < 0) {
                        cout<<"send msg to client failure!"<<endl;
                    }
                    close(clientfd);
                    continue;
                }

                int newfd = fork();
                if (newfd < 0) {
                    ERROR_HANDLE("create child process error!");
                } else if (newfd == 0) {
                    close(m_listenfd);
                    close(m_sigfd[0]);
                    close(m_sigfd[1]);
                    close(m_epollfd);
                    SubProcess(clientfd);
                    m_memmap->UnmapMemory();
                } else {
                    m_childpid.push_back(newfd);
                    m_curclitcount++;
                    close(clientfd);
                }
            } else if (currentfd == m_sigfd[0] && events[i].events == EPOLLIN) {
                char sigbuf[BUFSIZE];
                int sig_count = recv(currentfd, sigbuf, sizeof(sigbuf), 0);
                if (sig_count < 0) {
                    ERROR_HANDLE("receive signal error!");
                } else if (sig_count == 0) {
                    cout<<"the end of signal writing is closed"<<endl;
                    continue;
                } else {
                    int status;
                    for (int i = 0; i < sig_count; ++i) {
                        switch(sigbuf[i])
                        {
                            case SIGCHLD: {
                                              int pid;
                                              while ((pid = waitpid(-1, &status, 0)) > 0) {
                                                  m_curclitcount--;
                                                  RmSubPro(pid);
                                              }

                                              if (terminal && m_curclitcount == 0) {
                                                    server_stop = true;
                                              }
                                              break;
                                          }
                            case SIGINT:
                            case SIGTERM:
                                          {
                                            cout<<"kill all children!"<<endl;
                                            if (m_curclitcount == 0) {
                                                server_stop = true;
                                                break;
                                            }

                                            for (int i = 0; i < m_childpid.size(); ++i) {
                                                kill(m_childpid[i], SIGTERM);
                                            }

                                            terminal = true;
                                            break;
                                          }
                            default: break;
                        }
                    }
                }
            } else {

            }

        }
    }

    close(m_listenfd);
    cout<<"Client disconnect!"<<endl;

}


void HttpServer::SubProcess(int fd)
{
    int child_epoll = epoll_create(10);
    if (child_epoll < 0) {
        ERROR_HANDLE("create child epoll failure!");
    }

    AddfdToEpoll(child_epoll, fd);

    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int ret = epoll_wait(child_epoll, events, MAX_EVENTS, -1);

        if (ret < 0 && errno != EINTR) {
            ERROR_HANDLE("wait events error!");
        }

        char buffer[BUFSIZE];
        for (int i = 0; i < ret; ++i) {
            int socketfd = events[i].data.fd;
            if (socketfd == fd && events[i].events & EPOLLIN) {
                memset(buffer, 0, sizeof(buffer));

                int bytes_count;
                while ((bytes_count = recv(socketfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
                    cout<<"receive client msg: " <<buffer<<endl;
                    if (send(socketfd, buffer, strlen(buffer), 0) < 0) {
                        ERROR_HANDLE("send msg to client error!");
                    }
                    memset(buffer, 0, sizeof(buffer));
                }

                if (bytes_count == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        ERROR_HANDLE("receive msg from client error!");
                    }
                } else if (bytes_count == 0) {
                    cout<<"client end close the connection\n";
                    RmfdFromEpoll(child_epoll, socketfd);
                    close(socketfd);
                    break;
                }
            }
        }
    }

}

void HttpServer::RmSubPro(int pid)
{
    std::vector<int>::iterator it = find(m_childpid.begin(), m_childpid.end(), pid);
    m_childpid.erase(it);
}

void HttpServer::RmfdFromEpoll(int epollfd, int fd)
{
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        ERROR_HANDLE("remove fd from epoll failure!");
    }
}

void HttpServer::AddfdToEpoll(int epollfd, int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN|EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        ERROR_HANDLE("add fd to epoll error!");
    }

    SetfdNonBlocking(fd);

}

int HttpServer::SetfdNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void HttpServer::RegSignaler(int sig, void(*handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;
    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }

    sigfillset(&sa.sa_mask);
    if (sigaction(sig, &sa, NULL) != -1) {
        ERROR_HANDLE("register signal handler error!");
    }
}

void HttpServer::SignalHandler(int sig)
{
    int old_errno = errno;
    int msg = sig;

    if (send(m_sigfd[1],reinterpret_cast<char*>(&msg), 1, 0) < 0) {
        ERROR_HANDLE("send sig failure!");
    }
    errno = old_errno;
}

void HttpServer::SetupSignaler()
{
    RegSignaler(SIGCHLD, &HttpServer::SignalHandler);
    RegSignaler(SIGTERM, &HttpServer::SignalHandler);
    RegSignaler(SIGINT, &HttpServer::SignalHandler);
    RegSignaler(SIGPIPE, SIG_IGN);
}




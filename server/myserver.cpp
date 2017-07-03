#include "myserver.h"
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <cmath>
#define EPOLL_EVENTNUM 1024

void HttpServer::Start(int port)
{

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1) {
    }


    int on = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFU_SERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in)) == -1) {
        //    ERROR_EXIT("bind addr failure!");
    }


    if (listen(m_listenfd, DEFU_BACKLOG) == -1) {
        ERROR_EXIT("bind addr failure!");
    }

    cout<<"server listing..."<<endl;

    //1.
    //Echo_Select();
    //2.
    //Echo_Poll();
    //3.
    Echo_Epoll();
}

int HttpServer::SetFdNonBlock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void HttpServer::Echo_Select()
{
    cout<<"select implement..."<<endl;
    struct sockaddr_in client_addr;
    socklen_t clientaddr_len = sizeof(struct sockaddr_in);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO(&m_fdsetdata.readfds);
    FD_SET(m_listenfd, &m_fdsetdata.readfds);

    m_fdsetdata.client_fds.push_back(m_listenfd);
    m_fdsetdata.maxfd_index = 0;


    while (1) {
        m_fdsetdata.readyfds = m_fdsetdata.readfds;
        int ready_count = select(m_fdsetdata.client_fds[m_fdsetdata.maxfd_index] + 1, &m_fdsetdata.readyfds, NULL, NULL, &tv);
        if (ready_count == -1) {
            ERROR_EXIT("select fds failure!");
        } else if (ready_count > 0) {
            for (int i = 0; i < m_fdsetdata.client_fds.size(); ++i) {

                if (m_fdsetdata.client_fds[i] == m_listenfd && FD_ISSET(m_listenfd, &m_fdsetdata.readyfds)) {
                    int clientfd;
                    if ((clientfd = accept(m_listenfd, (struct sockaddr*)(&client_addr), &clientaddr_len)) < 0) {
                        ERROR_EXIT("accept client connection failure!");
                    }
                    cout<<clientfd<<" connect to client success!"<<endl;
                    m_fdsetdata.client_fds.push_back(clientfd);
                    m_fdsetdata.maxfd_index = m_fdsetdata.client_fds[m_fdsetdata.maxfd_index] > clientfd ? m_fdsetdata.maxfd_index:(m_fdsetdata.client_fds.size() - 1);
                    FD_SET(clientfd, &m_fdsetdata.readfds);
                } else if (FD_ISSET(m_fdsetdata.client_fds[i], &m_fdsetdata.readyfds)){
                    Echo(m_fdsetdata.client_fds[i], MULTIP_SELECT);
                }
            }
        }
    }
}

void HttpServer::Echo_Poll()
{
    cout<<"poll implement..."<<endl;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    struct pollfd  poll_ev;
    // vector<pollfd> event_queue;
    poll_ev.fd = m_listenfd;
    poll_ev.events = POLLIN;
    event_queue.push_back(poll_ev);

    while (1) {
        int ready_count = poll((struct pollfd*)(event_queue.data()), event_queue.size(),  -1);
        if (ready_count < 0) {
            ERROR_EXIT("poll ready events failure!");
        } else if (ready_count > 0) {
            int event_size = event_queue.size();
            for (int i = 0; i < event_size; ++i) {
                if (event_queue[i].revents & POLLIN) {
                    if (event_queue[i].fd == m_listenfd) {
                        int clientfd = accept(m_listenfd, (struct sockaddr*)(&client_addr), &addr_len);
                        if (clientfd < 0) {
                            ERROR_EXIT("receive connection from client failure!");
                        }
                        cout<<clientfd<<" connect to client success!"<<endl;
                        poll_ev.fd = clientfd;
                        poll_ev.events = POLLIN;
                        event_queue.push_back(poll_ev);
                    } else {
                        Echo(event_queue[i].fd, MULTIP_POLL);
                    }
                }
            }
        }
    }
}



void HttpServer::Echo_Epoll()
{
    cout<<"epoll implement..."<<endl;

    struct sockaddr_in clientaddr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    struct epoll_event ev, events[EPOLL_EVENTNUM];
    int epollfd;
    epollfd = epoll_create(EPOLL_EVENTNUM);

    if (epollfd < 0) {
        ERROR_EXIT("init epoll failure");
    }

    ev.data.fd = m_listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, m_listenfd, &ev) == -1) {
        ERROR_EXIT("fail to add event");
    }

    while (1) {
        int ret = epoll_wait(epollfd, (struct epoll_event*)events, EPOLL_EVENTNUM, -1);
        if (ret < 0) {
            ERROR_EXIT("wait ready events failure!");
        } else if (ret > 0) {
            for (int i = 0; i < ret; ++i) {
                if (events[i].events & EPOLLIN) {
                    if (events[i].data.fd == m_listenfd) {
                        int clientfd = accept(m_listenfd, (struct sockaddr*)(&clientaddr), &addr_len);
                        if (clientfd < 0) {
                            ERROR_EXIT("accept client connection fail");
                        }

                        ev.data.fd = clientfd;
                        ev.events = EPOLLIN | EPOLLET;
                        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev) == -1) {
                            ERROR_EXIT("epoll_ctl: add client EPOLLIN event failure!");
                        }
                        SetFdNonBlock(clientfd);
                    } else {
                        Echo(events[i].data.fd, MULTIP_EPOLL);
                    }
                }
            }
        }
    }

}

void HttpServer::FdCloseProcess(int fd, int flag)
{
    switch(flag) {
        case MULTIP_SELECT: {
                                FD_CLR(fd, &m_fdsetdata.readfds);
                                RemoveValue(m_fdsetdata.maxfd_index, m_fdsetdata.client_fds);
                                break;
                            }
        case MULTIP_POLL: {
                              for (int i = 0; i < event_queue.size(); ++i) {
                                  if (event_queue[i].fd == fd) {
                                      event_queue.erase(event_queue.begin() + i);
                                  }
                              }
                              break;
                          }
        case MULTIP_EPOLL: {
                               epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                               break;
                           }
        default:break;
    }
    close(fd);
}

void HttpServer::Echo(int clientfd, int flag)
{
    memset(m_buffer, 0, BUFSIZE);

    int ret = read(clientfd, m_buffer, BUFSIZE - 1);
    if (ret != -1) {
        if (ret == 0) {
            cout<<"client disconnect!"<<endl;

            FdCloseProcess(clientfd, flag);
        } else {
            cout<<m_buffer;
            ret = write(clientfd, m_buffer, strlen(m_buffer));
            if (ret == -1) {
                ERROR_EXIT("write data section to client error!");
            }

            if (flag == MULTIP_EPOLL) {
                while (1) {
                    memset(m_buffer, 0, BUFSIZE);
                    ret = read(clientfd, m_buffer, BUFSIZE - 1);
                    if (ret < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        }
                        ERROR_EXIT("read data section from client error!");
                    }

                    cout<<m_buffer;
                    ret = write(clientfd, m_buffer, strlen(m_buffer));
                    if (ret == -1) {
                        ERROR_EXIT("write data section to client error!");
                    }
                }

            }
        }
    }
    else {
        ERROR_EXIT("read data section from client error!");
    }
}

void  HttpServer::RemoveValue(int& index, vector<int> & fds)
{
    int first = -1, second = -1;
    for (int i = 0; i < fds.size(); ++i) {
        if (first == -1) {
            first = i;
            continue;
        }

        if (second == -1) {
            if (fds[first] < fds[i]) {
                first = i;
                second = first;
            } else {
                second = i;
            }
            continue;
        }

        if (fds[i] >= fds[first]) {
            second = first;
            first = i;
        } else if (fds[i] > fds[second]) {
            second = i;
        }
    }

    if (first != -1) fds.erase(fds.begin() + first);
    if (second != -1) {
        if (second < first)index = second;
        else index = second - 1;
    }

}



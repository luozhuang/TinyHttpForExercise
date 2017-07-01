#include "myserver.h"
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cmath>

void HttpServer::Start(int port)
{

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1) {
    }


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
    Echo_Select();
    //2.
    //
    //3.
}

void HttpServer::Echo_Select()
{
    struct sockaddr_in client_addr;
    socklen_t clientaddr_len;

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
                    Echo(m_fdsetdata.client_fds[i]);
                }
            }
        }
    }
}

void HttpServer::Echo_Poll()
{

}

void HttpServer::Echo_Epoll()
{

}

void HttpServer::Echo(int clientfd)
{
    memset(m_buffer, 0, BUFSIZE);

    int ret = read(clientfd, m_buffer, BUFSIZE - 1);
    if (ret != -1) {
        if (ret == 0) {
            cout<<"client disconnect!"<<endl;
            FD_CLR(clientfd, &m_fdsetdata.readfds);
            RemoveValue(m_fdsetdata.maxfd_index, m_fdsetdata.client_fds);
            close(clientfd);
        } else {
            cout<<m_buffer;
            ret = write(clientfd, m_buffer, BUFSIZE);
            if (ret == -1) {
                ERROR_EXIT("write data section to client error!");
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



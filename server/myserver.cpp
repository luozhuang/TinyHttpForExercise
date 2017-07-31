#include "myserver.h"

void HttpServer::Start(int port)
{

    m_lisentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lisentfd == -1) {
    }

    struct sockaddr_in server_addr, client_addr;
    socklen_t clientaddr_len;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFU_SERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_lisentfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in)) == -1) {
        //    ERROR_EXIT("bind addr failure!");
    }

    if (listen(m_lisentfd, DEFU_BACKLOG) == -1) {

    }

    cout<<"server listing..."<<endl;
    int clientfd;

    if ((clientfd = accept(m_lisentfd, (struct sockaddr*)(&client_addr), &clientaddr_len)) < 0) {
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
}

void HttpServer::RegSignaler(int sig, void(*handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;
    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }

    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void HttpServer::SignalHandler(int sig)
{
    int old_errno = errno;
    int msg = sig;

    if (send(m_sigfd[1],static_cast<char*>(&msg), 1, 0) < 0) {
        ERROR_HANDLE("send sig failure!");
    }
    errno = old_errno;
}

void SignalControl()
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigfd) == -1) {
        ERROR_HANDLE("create socket pipe error!");
    }

    RegSignaler(SIGCHLD, SignalHandler);
    RegSignaler(SIGTERM, SignalHandler);
    RegSignaler(SIGINT, SignalHandler);
    RegSignaler(SIGPIPE, SIG_IGN);
}




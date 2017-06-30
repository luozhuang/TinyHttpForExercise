#include "myserver.h"

void HttpServer::Start(int port)
{

    m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketfd == -1) {
    }

    struct sockaddr_in server_addr, client_addr;
    socklen_t clientaddr_len;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFU_SERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_socketfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in)) == -1) {
        //    ERROR_EXIT("bind addr failure!");
    }

    if (listen(m_socketfd, DEFU_BACKLOG) == -1) {

    }

    cout<<"server listing..."<<endl;
    int clientfd;

    if ((clientfd = accept(m_socketfd, (struct sockaddr*)(&client_addr), &clientaddr_len)) < 0) {
        ERROR_EXIT("connect failure!");
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
                    ERROR_EXIT("write data section to client error!");
                }
            }

        }
        else {
            ERROR_EXIT("read data section from client error!");
        }
    }

    close(clientfd);
    cout<<"Client disconnect!"<<endl;
}




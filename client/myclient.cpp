#include "myclient.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using namespace std;
#define DEFU_SERPORT 30023

void HttpClient::Connect(const char *addr, int remote_port)
{
    m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketfd == -1) {
        perror("create connection failure!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(struct sockaddr_in));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(DEFU_SERPORT);
    //remote_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, addr, &remote_addr.sin_addr);
    if (connect(m_socketfd, (struct sockaddr*)(&remote_addr), sizeof(struct sockaddr_in)) == -1) {
        perror("conect failure");
        exit(EXIT_FAILURE);
    }

    cout<<"connect to server success!"<<endl;

    int ret;
    do {
        memset(m_buffer, 0, BUFSIZE);
        ret = read(0, m_buffer, BUFSIZE);
        if (ret == -1) {

            perror("input data error!");
            exit(EXIT_FAILURE);
        } if (ret == 0) {
            cout<<"No data need transmit!"<<endl;
            break;
        }

        ret = write(m_socketfd, m_buffer, BUFSIZE);
        if (ret == -1) {
            perror("send data section to server error!");
            exit(EXIT_FAILURE);
        } else {
            memset(m_buffer, 0, BUFSIZE);
            ret = read(m_socketfd, m_buffer, BUFSIZE);
            if (ret == -1) {
                perror("receive data from server error!");
                exit(EXIT_FAILURE);
            } else {
                cout<<"The data section from server:"<<m_buffer;
            }
        }
    } while (ret != 0);
    cout<<"End client connection!"<<endl;
    sleep(2);
    close(m_socketfd);
}



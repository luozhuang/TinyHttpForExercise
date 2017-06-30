#ifndef __MYCLIENT_H__
#define __MYCLIENT_H__
#define NAMELEN  25
#define BUFSIZE 1024

class HttpClient
{

public:

    HttpClient()
    {

    }
    ~HttpClient()
    {

    }
    void Connect(const char *remote_addr = "127.0.0.1", int remote_port = 30023);
private:
    int m_socketfd;
    char m_buffer[BUFSIZE];
    //char name[NAMELEN];
};
#endif

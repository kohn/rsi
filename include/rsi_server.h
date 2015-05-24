#ifndef RSI_SERVER_H
#define RSI_SERVER_H
#include "sysinfo.h"
class RsiServer
{
private:
    SysInfo *sysinfo;
    static void sig_child(int signo);
public:
    RsiServer(int port, SysInfo *sysinfo);
    int listen_port(int port);
    int accept_client(int server_sockfd);
    int communicate(int client_sockfd);
    virtual ~RsiServer(){}
};
#endif

#ifndef RSI_SERVER_H
#define RSI_SERVER_H
#include "sysinfo.h"
#include "virt.h"
class RsiServer
{
private:
    SysInfo *sysinfo;
    VM_Controller vm_controller;
    static void sig_child(int signo);
    static void sig_int(int signo);
public:
    RsiServer(int port, SysInfo *sysinfo);
    int listen_port(int port);
    int accept_client(int server_sockfd);
    int communicate(int client_sockfd);
    virtual ~RsiServer(){}
};
#endif

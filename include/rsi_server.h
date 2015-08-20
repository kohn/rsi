#ifndef RSI_SERVER_H
#define RSI_SERVER_H
#include "sysinfo.h"
#include "virt.h"
#include <vector>
class RsiServer
{
private:
    SysInfo *_sysinfo;
    VM_Controller _vm_controller;
    static void sig_child(int signo);
    static void sig_int(int signo);
    int response_host_mem_usage(int client_sockfd);
    int response_host_node_info(int client_sockfd);
    int response_host_cpu_usage(int client_sockfd);
    int response_vm_info(int client_sockfd);
    int response_vm_detail_by_name(std::vector<std::string> &strings,
                                   int client_sockfd);
    int response_open_vm(std::vector<std::string> &strings,
                         int client_sockfd);
    int response_close_vm(std::vector<std::string> &strings,
                          int client_sockfd);
    int response_vm_ip(std::vector<std::string> &strings,
                       int client_sockfd);
    int response_port_forward(std::vector<std::string> &strings,
                              int client_sockfd);
    int response_image_fd_by_name(std::vector<std::string> &strings,
                                  int client_sockfd);
    int response_image_by_fd(std::vector<std::string> &strings,
                             int client_sockfd);
    int response_config_by_fd(std::vector<std::string> &strings,
                             int client_sockfd);
    int response_file_by_fd(std::vector<std::string> &strings,
                             int client_sockfd);
    int response_fetch_image_by_name(std::vector<std::string> &strings,
                                     int client_sockfd);
    int response_job_progress(std::vector<std::string> &strings,
                              int client_sockfd);
    int response_new_vm_config(std::vector<std::string> &strings,
                               int client_sockfd);
    std::string _make_cmd_size_mismatch_msg(std::string cmd, int size_wanted);
public:
    RsiServer(int port, SysInfo *sysinfo);
    int listen_port(int port);
    int accept_client(int server_sockfd);
    int communicate(int client_sockfd);
    virtual ~RsiServer(){}
};
#endif

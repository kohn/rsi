#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <cerrno>
#include <arpa/inet.h>
#include <set>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include "rsi_server.h"
#include "sysinfo.h"
#include "globals.h"
#include "tools.h"
#include <vector>
#include "core.h"

RsiServer::RsiServer(int port, SysInfo *sysinfo){
    this->_sysinfo = sysinfo;
    if(signal(SIGCHLD, sig_child) == SIG_ERR){
        LOG_ERROR("could not bind SIGCHLD to sig_child");
        exit(-1);
    }
    if(signal(SIGINT, sig_int) == SIG_ERR){
        LOG_ERROR("could not bind SIGINT to sig_int");
        exit(-1);
    }
    int server_sockfd = listen_port(port);
    int keep_alive = 1;
    if(setsockopt(server_sockfd, SOL_SOCKET,
                  SO_KEEPALIVE, (void *)&keep_alive,
                  sizeof(keep_alive)) == -1){
        LOG_ERROR("Could not set keep_alive option");
        exit(-1);
    }
    while(1){
        int client_sockfd = accept_client(server_sockfd);
        pid_t pid = fork();
        if(pid == 0){ // child process
            close(server_sockfd);
            while (true) {
                if(communicate(client_sockfd) < 0)
                    exit(-1);
            }
        }
        else{
            close(client_sockfd);
        }
    }
}

void RsiServer::sig_child(int signo){
    signo = signo;
    LOG_INFO("Connection Closed");
}

void RsiServer::sig_int(int signo){
    signo = signo;
    LOG_INFO("SIGINT captured");
    exit(0);
}
int RsiServer::listen_port(int port){
    int sockfd;
    struct sockaddr_in server_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(-1);
    }

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int opt=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) < 0)
    {
        perror("bind error");
        exit(-1);
    }
    /* listen: 监听绑定的端口 */
    if(listen(sockfd, 1) < 0){
        perror("listen error");
        exit(-1);
    }
    return sockfd;
}
int RsiServer::accept_client(int server_sockfd){
    int client_sockfd;
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if( (client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &sin_size)) < 0)
        perror("accept error");
    return client_sockfd;
}

int RsiServer::communicate(int client_sockfd){
    char buf[4096];
    try{
        int read_num = Networking::read_msg(client_sockfd, buf);
        buf[read_num] = '\0';
        LOG_INFO(buf);
    }
    catch(NetworkError){
        LOG_ERROR("Network Error");
        return -1;
    }
    catch(LocalError){
        LOG_ERROR("Local Error");
        return -1;
    }
    
    
    std::vector<std::string> strings;
    std::string str(buf);
    Tools::split(str, strings, ' ');
    if(strings.size() == 0){
        LOG_ERROR("could not get any valid string from socket READ");
        return -1;
    }

    try{
        if(strings[0] == "GET_HOST_MEM_USAGE"){
            return response_host_mem_usage(client_sockfd);
        }
        if(strings[0] == "GET_HOST_NODE_INFO"){
            return response_host_node_info(client_sockfd);
        }
        if(strings[0] ==  "GET_VM_INFO"){
            return response_vm_info(client_sockfd);
        }
        if(strings[0] == "GET_HOST_CPU_USAGE"){
            return response_host_cpu_usage(client_sockfd);
        }
        if(strings[0] == "GET_VM_DETAIL_BY_NAME"){
            return response_vm_detail_by_name(strings, client_sockfd);
        }
        if(strings[0] == "OPEN_VM"){
            return response_open_vm(strings, client_sockfd);
        }
        if(strings[0] == "CLOSE_VM"){
            return response_close_vm(strings, client_sockfd);
        }
        if(strings[0] == "GET_VM_IP"){
            return response_vm_ip(strings, client_sockfd);
        }
        if(strings[0] == "PORT_FORWARD"){
            return response_port_forward(strings, client_sockfd);
        }
        // for other rsi_server B to get the image fd specified by vm_name on A
        if(strings[0] == "GET_IMAGE_FD_BY_NAME"){
            return response_image_fd_by_name(strings, client_sockfd);
        }
        // for other rsi_server B to get the image specified by fd on A
        if(strings[0] == "GET_IMAGE_BY_FD"){
            return response_image_by_fd(strings, client_sockfd);
        }
        // fetch image from another rsi_server
        if(strings[0] == "FETCH_IMAGE_BY_NAME"){
            return response_fetch_image_by_name(strings, client_sockfd);
        }
        if(strings[0] == "GET_JOB_PROGRESS"){
            return response_job_progress(strings, client_sockfd);
        }
        if(strings[0] == "NEW_VM_CONFIG"){
            return response_new_vm_config(strings, client_sockfd);
        }
        std::string response = "{\"status\": \"cmd not recognized\"}";
        Networking::write(client_sockfd, response.c_str(), response.length());
    }
    catch(NetworkError &e){
        LOG_ERROR("Network Error");
        return -1;
    }
    catch(LocalError &e){
        LOG_ERROR("Local Error");
        return -1;
    }
    return 0;
}

int RsiServer::response_host_mem_usage(int client_sockfd){
    std::string response = _sysinfo->get_host_mem_usage();
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_host_node_info(int client_sockfd){
    std::string response = _sysinfo->get_host_node_info();
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_host_cpu_usage(int client_sockfd){
    std::string response = _sysinfo->get_host_cpu_usage();
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}

int RsiServer::response_vm_info(int client_sockfd){
    std::string response = _vm_controller.get_vm_info();
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}

int RsiServer::response_vm_detail_by_name(std::vector<std::string> &strings,
                                          int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("GET_VM_DETAIL_BY_NAME", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    response = _vm_controller.get_vm_detail_by_name(strings[1]);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_open_vm(std::vector<std::string> &strings,
                                int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("OPEN_VM", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    response = _vm_controller.open_vm(strings[1]);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_close_vm(std::vector<std::string> &strings,
                                 int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("CLOSE_VM", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    int vm_id = atoi(strings[1].c_str());
    response = _vm_controller.close_vm(vm_id);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_vm_ip(std::vector<std::string> &strings,
                              int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("GET_VM_IP", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    std::string vm_name = strings[1].c_str();
    response = _vm_controller.get_vm_ip_by_name(vm_name);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_port_forward(std::vector<std::string> &strings,
                                     int client_sockfd){
    std::string response;
    if(strings.size() != 4){
        response = _make_cmd_size_mismatch_msg("PORT_FORWARD", 4);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    std::string host_ip_address = strings[1];
    std::string guest_ip_address = strings[2];
    std::string guest_port = strings[3];
    response = _vm_controller.port_forward(host_ip_address,
                                           guest_ip_address,
                                           guest_port);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_image_fd_by_name(std::vector<std::string> &strings,
                                         int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("GET_IMAGE_FD_BY_NAME", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    std::stringstream s;
    std::string vm_name = strings[1];
    // get fd but do not close it. close it after reading.
    int fd = _vm_controller.get_image_fd_by_name(vm_name);
    long long filesize = _vm_controller.get_image_size_by_name_in_ll(vm_name);
    s << fd << " " << filesize;
    DEBUG(s.str());
    Networking::write(client_sockfd, s.str().c_str(), s.str().length());
    return 0;
}

// always return -1 after sending image, so that to close the socket
int RsiServer::response_image_by_fd(std::vector<std::string> &strings,
                                    int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("GET_IMAGE_BY_FD", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    int fd = atoi(strings[1].c_str());
    char buffer[4096];
    while(1){
        int read_count = read(fd, buffer, 4096);
        if(read_count < 0){
            perror("read error");
            response = "{\"status\": \"read fd error\"}";
            Networking::write(client_sockfd, response.c_str(), response.length());
            return -1;
        }
        else if(read_count == 0)
            break;
        Networking::write(client_sockfd, buffer, read_count);
    }
    response = "{\"status\": \"ok\"}";
    Networking::write(client_sockfd, response.c_str(), response.length());
    close(fd);
    return -1;
}
int RsiServer::response_fetch_image_by_name(std::vector<std::string> &strings,
                                            int client_sockfd){
    std::string response;
    if(strings.size() != 5){
        response = _make_cmd_size_mismatch_msg("FETCH_IMAGE_BY_NAME", 5);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    std::string host_ip = strings[1];
    int rsi_server_port = atoi(strings[2].c_str());
    std::string vm_name = strings[3];
    std::string new_name = strings[4];
    response = _vm_controller.fetch_image_by_name(host_ip, rsi_server_port, vm_name, new_name);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_job_progress(std::vector<std::string> &strings,
                                     int client_sockfd){
    std::string response;
    if(strings.size() != 2){
        response = _make_cmd_size_mismatch_msg("GET_JOB_PROCESS", 2);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    int job_id = atoi(strings[1].c_str());
    response = _vm_controller.get_job_progress(job_id);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}
int RsiServer::response_new_vm_config(std::vector<std::string> &strings,
                                      int client_sockfd){
    std::string response;
    if(strings.size() != 5){
        response = _make_cmd_size_mismatch_msg("NEW_VM_CONFIG", 5);
        Networking::write(client_sockfd, response.c_str(), response.length());
        return -1;
    }
    std::string vm_name = strings[1];
    std::string img_path = strings[2];
    int cpu_num = atoi(strings[3].c_str());
    int memory_in_mb = atoi(strings[4].c_str());
    response = _vm_controller.new_vm_config(vm_name, img_path, cpu_num, memory_in_mb);
    DEBUG(response);
    Networking::write(client_sockfd, response.c_str(), response.length());
    return 0;
}

std::string RsiServer::_make_cmd_size_mismatch_msg(std::string cmd, int size_wanted){
    std::stringstream ss;                                           
    ss << cmd << " needs " << size_wanted - 1 << " arguments";
    LOG_INFO(ss.str());
    std::string response;
    response = "{\"status\": \"";
    response += ss.str() + "\"}";
    return response;
}

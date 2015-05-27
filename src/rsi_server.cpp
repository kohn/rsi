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

RsiServer::RsiServer(int port, SysInfo *sysinfo){
    this->sysinfo = sysinfo;
    int server_sockfd = listen_port(port);
    if(signal(SIGCHLD, sig_child) == SIG_ERR){
        LOG_ERROR("could not bind SIGCHLD to sig_child");
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
    int read_num;
    char buf[4096];
again:
    if((read_num = read(client_sockfd, buf, 4095))> 0){
        buf[read_num] = '\0';
        LOG_INFO(buf);
        std::vector<std::string> strings;
        std::string str(buf);
        Tools::split(str, strings, ' ');
        if(strings.size() == 0){
            LOG_ERROR("could not get any valid string from socket READ");
            return -1;
        }
        if(strings[0] == "GET_HOST_MEM_USAGE"){
            std::string response = sysinfo->get_host_mem_usage();
            DEBUG(response);
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
        else if(strings[0] == "GET_HOST_NODE_INFO"){
            std::string response = sysinfo->get_host_node_info();
            DEBUG(response);
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
        else if(strings[0] ==  "GET_VM_INFO"){
            std::string response = sysinfo->get_vm_info();
            DEBUG(response);
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
        else if(strings[0] == "GET_HOST_CPU_USAGE"){
            std::string response = sysinfo->get_host_cpu_usage();
            DEBUG(response);
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
        else if(strings[0] == "GET_VM_DETAIL"){
            if(strings.size() == 1){
                LOG_ERROR("GET_VM_DETAIL needs 2 arguments");
                return -1;
            }
            int vm_id = atoi(strings[1].c_str());
            std::string response = sysinfo->get_vm_detail(vm_id);
            DEBUG(response);
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
        else {
            std::string response = "{\"status\": \"cmd not recognized\"}";
            if(write(client_sockfd, response.c_str(), response.length()) < 0){
                perror("write error");
                return -1;
            }
        }
            
    }
    else if(read_num <0 && errno==EINTR){
        perror("read interrupt");
        goto again;
    }
    else if(read_num < 0){
        perror("read error");
        return -1;
    }
    else{
        close(client_sockfd);
        return -1;
    }
    return 0;
}

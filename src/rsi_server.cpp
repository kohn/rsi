#include "get_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
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

VekaServer::VekaServer(int port, SysInfo *sysinfo){
    sysinfo = sysinfo;
    int server_sockfd = listen_port(port);
    while(1){
        int client_sockfd = accept_client(server_sockfd);
        pid_t pid = fork();
        if(pid == 0){ // child process
            close(server_sockfd);
            while (true) {
                if(communicate(client_sockfd) < 0)
                    exit(-1);
                //sleep(time_interval);
            }
        }
        else{
            close(client_sockfd);
            std::cout << "pid: " << pid << std::endl;
            wait(NULL);   // 只允许一个连接
            std::cout << "child exit" << std::endl;
        }
    }
}

int VekaServer::listen_port(int port){
    int sockfd;
    struct sockaddr_in server_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
    }

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) < 0)
    {
        perror("bind error");
    }
    /* listen: 监听绑定的端口 */
    if(listen(sockfd, 1) < 0)
        perror("listen error");

    return sockfd;
}
int VekaServer::accept_client(int server_sockfd){
    int client_sockfd;
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if( (client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &sin_size)) < 0)
        perror("accept error");
    return client_sockfd;
}

int VekaServer::communicate(int client_sockfd){
    int read_num;
    char buf[4096];
again:
    if((read_num = read(client_sockfd, buf, 4095))> 0){
        buf[read_num] = '\0';

        if(strcmp(buf, "GET_HOST_MEM_USAGE") == 0){
            std::string response = sysinfo->get_host_mem_usage();
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

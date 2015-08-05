#include "rsi_client.h"
#include <sys/socket.h>
#include <cerrno>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
RSIClient::RSIClient(std::string ip, int port){
    _good = 0;
    
    struct sockaddr_in servaddr;

    if( (_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        _good = -1;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr) < 0) {
        _good = -2;
    }

    if(connect(_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        _good = -3;
    }
}


RSIClient::~RSIClient(){
    if(_good >= 0){
        close(_sockfd);
    }
}

std::string RSIClient::communicate(std::string msg){
    LOG_INFO(msg);
    if(write(_sockfd, msg.c_str(), msg.length()) < 0){
        LOG_ERROR("write error");
        return "";
    }
    
    char buf[4096];
    std::stringstream ss;

    int read_num;
    if( (read_num = read(_sockfd, buf, 4095)) != 0) {
        if(read_num < 0)
            LOG_ERROR("read error");
        buf[read_num] = '\0';
        ss << buf;
    }
    return ss.str();
}

int RSIClient::good(){
    return _good >= 0;
}


// send msg to remote server, save response to a file named 'filename'
int RSIClient::download(std::string msg, std::string filename, long filesize){
    LOG_INFO(msg);
    if(write(_sockfd, msg.c_str(), msg.length()) < 0){
        LOG_ERROR("write error");
        return -1;
    }
    int fd = open(filename.c_str(), O_WRONLY|O_CREAT);
    LOG_INFO(filename);
    if(fd < 0){
        perror("rsi_client.cpp");
        return -1;
    }
    
    char buf[4096];
    int read_count;
    long total_count = 0;
    while(total_count < filesize){
        read_count = read(_sockfd, buf, 4096);
        // read erro
        if(read_count < 0){
            close(fd);
            return -1;
        }
        else if(read_count == 0){
            if(total_count != filesize){
                return -1;
            }
            else
                break;
        }
        // write to file error
        if(write(fd, buf, read_count) != read_count){
            close(fd);
            return -1;
        }
        total_count += read_count;
    }
    close(fd);
    return 0;
}

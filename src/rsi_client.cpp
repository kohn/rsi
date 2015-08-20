#include "rsi_client.h"
#include <sys/socket.h>
#include <cerrno>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include "globals.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "core.h"
#define BUF_SIZE 4096

RSIClient::RSIClient(std::string ip, int port):_image_size(0),
                                               _image_size_downloaded(0),
                                               _downloading_xml(true),
                                               _dead(true){
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
    DEBUG(_sockfd)
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
    LOG_INFO(ss.str());
    return ss.str();
}

int RSIClient::good(){
    return _good >= 0;
}


int RSIClient::download_img(std::string vm_name, std::string filename){
    
    // ask remote server to open image file and return its fd & size
    std::string msg = "GET_IMAGE_FD_BY_NAME ";
    msg += vm_name;
    std::string fd_and_size = communicate(msg);
    std::stringstream ss(fd_and_size);
    std::string fd;
    std::string fd_config;
    long long filesize;
    long long filesize_config;
    ss >> fd;
    if( fd == "-1"){
        LOG_ERROR("remote server could not open image file");
        return -1;
    }
    ss >> filesize;

    ss >> fd_config;
    if( fd_config == "-1"){
        LOG_ERROR("remote server could not open config file");
        return -1;
    }
    ss >> filesize_config;

    _image_size = filesize;

    // ask remote server to begin sending xml
    msg = "GET_CONFIG_BY_FD ";
    msg += fd_config;
    try{
        Networking::write(_sockfd, msg.c_str(), msg.length());
    }
    catch(NetworkError &e){
        LOG_ERROR("Downloading Image: Network Error");
    }
    catch(LocalError &e){
        LOG_ERROR("Downloading Image: Local Error");
    }
    int fd_write = open((filename+".xml").c_str(), O_WRONLY|O_CREAT);
    long long filesize_config_downloaded;
    _do_download(fd_write, filesize_config, filesize_config_downloaded);
    close(fd_write);
    _downloading_xml = false;

    // ask remote server to begin sending image
    msg = "GET_IMAGE_BY_FD ";
    msg += fd;
    try{
        Networking::write(_sockfd, msg.c_str(), msg.length());
    }
    catch(NetworkError &e){
        LOG_ERROR("Downloading Image: Network Error");
    }
    catch(LocalError &e){
        LOG_ERROR("Downloading Image: Local Error");
    }
    fd_write = open(filename.c_str(), O_WRONLY|O_CREAT);
    _do_download(fd_write, _image_size, _image_size_downloaded);
    close(_sockfd);
    close(fd_write);
    _dead = true;

    return 0;
}
    

void RSIClient::_do_download(int fd_write, long long size,
                             long long &size_downloaded){
    char buf[BUF_SIZE];
    int read_count;
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    while(size_downloaded < size){
        int next_read = BUF_SIZE;
        if(size - size_downloaded < BUF_SIZE)
            next_read = (int)(size - size_downloaded);
        
        read_count = read(_sockfd, buf, next_read);
        // read erro
        if(read_count < 0){
            return;
        }
        else if(read_count == 0){
            if(size_downloaded != size){
                return;
            }
            else
                break;
        }
        // write to file error
        if(write(fd_write, buf, read_count) != read_count){
            return;
        }
        size_downloaded += read_count;
    }
    
    fchmod(fd_write, mode);
    return;
}

int RSIClient::download_progress(){
    if(_downloading_xml)
        return 0;
    return (int)((_image_size_downloaded*1.0/_image_size)*100);
}

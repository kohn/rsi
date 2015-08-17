#include "rsi_client.h"
#include <sys/socket.h>
#include <cerrno>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
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
    LOG_INFO(ss.str());
    return ss.str();
}

int RSIClient::good(){
    return _good >= 0;
}


int RSIClient::download_img(int fd_write, std::string vm_name, int job_id){
    // ask remote server to open image file and return its fd & size
    std::string msg = "GET_IMAGE_FD_BY_NAME ";
    msg += vm_name;
    std::string fd_and_size = communicate(msg);
    std::stringstream ss(fd_and_size);
    std::string fd;
    long long filesize;
    ss >> fd;
    if( fd == "-1"){
        LOG_ERROR("remote server could not open image file");
        return -1;
    }
    ss >> filesize;

    // register download info
    long long *download_info = new long long[2];
    download_info[0] = filesize;
    download_info[1] = 0LL;
    jobid_downloadinfo_map[job_id] = download_info;

    // ask remote server to begin sending data
    msg = "GET_IMAGE_BY_FD ";
    msg += fd;
    if(write(_sockfd, msg.c_str(), msg.length()) < 0){
        LOG_ERROR("write error");
        return -1;
    }

    // create a new thread to download
    pthread_t pid;
    download_args *dargs = new download_args;
    dargs->fd_read = _sockfd;
    dargs->fd_write = fd_write;
    dargs->download_info = download_info;
    dargs->start_pos = 0ll;
    if(pthread_create(&pid, NULL, _do_download, (void *)dargs) == 0){
        return 0;
    }
    else{
        return -1;
    }
}

void * RSIClient::_do_download(void* args){
    download_args * dargs = (download_args *)args;
    char buf[4096];
    int read_count;
    long long total_count = dargs->start_pos;

    lseek(dargs->fd_write, dargs->start_pos, SEEK_SET);
    while(total_count < dargs->download_info[0]){
        read_count = read(dargs->fd_read, buf, 4096);
        // read erro
        if(read_count < 0){
            return NULL;
        }
        else if(read_count == 0){
            if(total_count != dargs->download_info[0]){
                return NULL;
            }
            else
                break;
        }
        // write to file error
        if(write(dargs->fd_write, buf, read_count) != read_count){
            return NULL;
        }
        total_count += read_count;
        dargs->download_info[1] = total_count;
    }
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    fchmod(dargs->fd_write, mode);
    close(dargs->fd_write);
    delete dargs;
    close(dargs->fd_read);
    return NULL;
}

int RSIClient::download_progress(int job_id){
    long long * download_info = jobid_downloadinfo_map[job_id];
    long long downloaded = download_info[1];
    long long filesize = download_info[0];
    return (int)(downloaded*100/filesize);
}

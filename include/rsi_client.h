#ifndef RSI_CLIENT_H
#define RSI_CLIENT_H
#include <string>
#include "globals.h"
#include <map>
typedef struct{
        int fd_read;
        int fd_write;
        long long *download_info;
        long long start_pos;    // file position start to write
} download_args;

class RSIClient
{
private:
    int _sockfd;
    int _good;
    std::map<int, long long*> jobid_downloadinfo_map;
    static void * _do_download(void* args);
    
    
public:
    RSIClient(std::string ip, int port);
    std::string communicate(std::string msg);
    int download_img(int fd_write, std::string vm_name, int job_id);
    int download_progress(int job_id);
    int good();
    ~RSIClient();
};
#endif

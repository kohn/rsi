#ifndef RSI_CLIENTS_H
#define RSI_CLIENTS_H
#include <string>
#include <map>
#include "rsi_client.h"
#include <vector>

// an RSI clients manager
// mainly used to download VM images from other RSI servers
class RSIClients{

private:
    std::map<std::string, RSIClient*> clients;
    std::string _make_client_id(std::string ip, int port);
    
    
    RSIClient * _new_client(std::string ip, int port);
    
    RSIClients(){}               // make this class as singleton
    RSIClients(const RSIClients&);
    RSIClients& operator=(const RSIClients&);

    std::vector<RSIClient*> client_by_jobid;
    
    struct download_args{
        int fd_read;
        int fd_write;
        int job_id;
        long long start_pos;    // file position start to write
        long long filesize;
    };
    
    // this function would be called in a new thread
    void * _do_download(void* args);
    
public:
    // signleton pattern
    static RSIClients * get_instance();
    
    // arguments:
    // select where to download, which img to download, 
    // and what file name the downloaded img should be.
    // 
    // return:
    // return a job id, which could be used to track the job
    int download_img(std::string ip, int port,
                     std::string vm_name,
                     std::string filename);

    // argument:
    // the download job id
    //
    // return:
    // progress in [0-100]%
    int download_progress(int job_id);

    ~RSIClients();
};
#endif

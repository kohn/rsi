#ifndef RSI_CLIENTS_H
#define RSI_CLIENTS_H
#include <string>
#include <map>
#include "rsi_client.h"
#include <vector>

typedef struct{
    char *ip;
    int port;
    char *vm_name;
    char *filename;
} download_args;


// an RSI clients manager
// mainly used to download VM images from other RSI servers
class RSIClients{

private:
    RSIClients(){}               // make this class as singleton
    RSIClients(const RSIClients&);
    RSIClients& operator=(const RSIClients&);

    static std::vector<RSIClient*> client_by_jobid;
    static std::vector<int> jobs_progress;
        
    // this function would be called in a new thread
    static void * _do_download_img(void* args);
    static void _do_download();
    
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

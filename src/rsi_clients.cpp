#include "rsi_clients.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "tools.h"
#include "rsi_client.h"
#include <string.h>


std::vector<RSIClient*> RSIClients::client_by_jobid;

RSIClients * RSIClients::get_instance(){
    static RSIClients _instance;
    return &_instance;
}

int RSIClients::download_img(std::string ip, int port,
                             std::string vm_name,
                             std::string filename){
    int job_id = client_by_jobid.size();


    download_args *dargs = new download_args;
    dargs->ip = new char[ip.length()];
    strcpy(dargs->ip, ip.c_str());

    dargs->port = port;
    dargs->vm_name = new char[vm_name.length()];
    strcpy(dargs->vm_name, vm_name.c_str());

    dargs->filename = new char[filename.length()];
    strcpy(dargs->filename, filename.c_str());

    pthread_t pid;
    if(pthread_create(&pid, NULL, _do_download_img, (void *)dargs) == 0){
        return job_id;
    }
    else{
        return -1;
    }
}

int RSIClients::download_progress(int job_id){
    if( (unsigned int)job_id >= client_by_jobid.size()){
        return -1;
    }
    RSIClient * client = client_by_jobid[job_id];
    return client->download_progress();
}

RSIClients::~RSIClients(){
    std::vector<RSIClient*>::iterator it;
    for(it = client_by_jobid.begin(); it!=client_by_jobid.end(); it++){
        delete (*it);
    }
}

// ============================================================ private functions

void * RSIClients::_do_download_img(void * args){
    download_args *dargs  = (download_args*)args;
    std::string ip(dargs->ip);
    std::string vm_name(dargs->vm_name);
    std::string filename(dargs->filename);
    int port = dargs->port;
    
    RSIClient *client = new RSIClient(ip, port);
    client_by_jobid.push_back(client);

    if(client->download_img(vm_name, filename) < 0)
        LOG_ERROR("download fail");

    return NULL;
}

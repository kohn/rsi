#include "rsi_clients.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "tools.h"
#include "rsi_client.h"

RSIClients * RSIClients::get_instance(){
    static RSIClients _instance;
    return &_instance;
}

int RSIClients::download_img(std::string ip, int port,
                             std::string vm_name,
                             std::string filename){
    int job_id = client_by_jobid.size();

    // get the RSIClient
    std::string client_id = _make_client_id(ip, port);
    RSIClient *client = _new_client(ip, port);
    client_by_jobid.push_back(client);

    int fd_write = open(filename.c_str(), O_WRONLY|O_CREAT);
    client->download_img(fd_write, vm_name, job_id);
    
    return job_id;
}

int RSIClients::download_progress(int job_id){
    if( job_id >= client_by_jobid.size()){
        return -1;
    }
        
    RSIClient * client = client_by_jobid[job_id];
    return client->download_progress(job_id);
}

RSIClients::~RSIClients(){
    std::map<std::string, RSIClient*>::iterator it;
    for(it = clients.begin(); it!=clients.end(); it++){
        delete it->second;
    }
}

// ============================================================ private functions

std::string RSIClients::_make_client_id(std::string ip, int port){
    return (ip + Tools::to_string(port, 10));
}

// create a new client, add to clients and return the new client
RSIClient * RSIClients::_new_client(std::string ip, int port){
    RSIClient *client = new RSIClient(ip, port);
    std::string client_id = _make_client_id(ip, port);
    clients[client_id] = client;
    return client;
}

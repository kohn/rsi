#ifndef RSI_CLIENT_H
#define RSI_CLIENT_H
#include <string>
#include "globals.h"
#include <map>

class RSIClient
{
private:
    int _sockfd;
    int _good;
    long long _image_size;
    long long _image_size_downloaded;
    bool _downloading_xml;
    bool _dead;
    void _do_download(int fd_write, long long size, long long &size_dowloaded);
    std::string communicate(std::string msg);
    
public:
    RSIClient(std::string ip, int port);
    int download_img(std::string vm_name, std::string filename);
    int download_progress();
    bool alive(){
        return !_dead;
    }
    int good();
    ~RSIClient();
};
#endif

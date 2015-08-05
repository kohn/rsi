#ifndef RSI_CLIENT_H
#define RSI_CLIENT_H
#include <string>
#include "globals.h"
class RSIClient
{
private:
    int _sockfd;
    int _good;
    
public:
    RSIClient(std::string ip, int port);
    std::string communicate(std::string msg);
    int download(std::string msg, std::string filename, long filesize);
    int good();
    ~RSIClient();
};
#endif

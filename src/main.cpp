#include "get_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <arpa/inet.h>
#include "sysinfo.h"
#include <set>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include "rsi_server.h"
#include "sysinfo.h"

int main(int argc, char *argv[])
{
    char *cmd;
    if((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    // TODO: debug模式先不daemon
    // daemonize(cmd);

    int port = 7209;

    std::map<std::string, std::string> config;
    if(!ReadConfig("rsi_server.config", config)){
        std::map<std::string, std::string>::iterator it = config.find("port");
        if(it != config.end())
            port = atoi((it->second).c_str());
    }

    SysInfo sysinfo;
    RsiServer rsi_server(port, &sysinfo);
    
    return 0;
}


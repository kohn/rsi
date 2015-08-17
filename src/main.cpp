#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <arpa/inet.h>
#include <set>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include "rsi_server.h"
#include "native_implement_sysinfo.h"
#include "tools.h"
#include "globals.h"

int main(int argc, char *argv[])
{

    int port = 7209; 

    std::map<std::string, std::string> config;
    Tools tools;
    if(tools.ReadConfig("rsi.config", config)){
        std::map<std::string, std::string>::iterator it = config.find("port");
        if(it != config.end()){
            port = atoi((it->second).c_str());
        }
    }
    
    if(argc == 2 && strcmp(argv[1], "-d") == 0){
        daemon(0, 0);
    }

    NativeImplementSysInfo native_sysinfo;
    RsiServer rsi_server(port, &native_sysinfo);
    
    return 0;
}


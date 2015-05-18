#ifndef SYSINFO_H
#define SYSINFO_H
#include <string>
#include "vsf.h"
class SysInfo
{
private:
    void get_mem_info(long long *total_mem, long long *free_mem);
    Vsf* framework;
    Host *host;
public:
    SysInfo();
    std::string get_host_mem_usage();
    std::string get_host_node_info();
    std::string get_vm_info();
    virtual ~SysInfo(){}
};
#endif

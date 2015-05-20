#ifndef NATIVE_IMPLEMENT_SYSINFO_H
#define NATIVE_IMPLEMENT_SYSINFO_H
#include "sysinfo.h"
#include <vector>
#include <map>

class NativeImplementSysInfo : public SysInfo
{
private:
    void get_mem_info(long long *total_mem, long long *free_mem);
    void get_node_info(std::vector<std::map<std::string, std::vector<int> > > &nodes);
public:
    NativeImplementSysInfo();
    std::string get_host_mem_usage();
    std::string get_host_node_info();
    std::string get_vm_info();
    ~NativeImplementSysInfo(){}
};

#endif // NATIVE_IMPLEMENT_SYSINFO_H

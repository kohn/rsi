#ifndef NATIVE_IMPLEMENT_SYSINFO_H
#define NATIVE_IMPLEMENT_SYSINFO_H
#include "sysinfo.h"
#include <vector>
#include <map>
#include <libvirt/libvirt.h>

class NativeImplementSysInfo : public SysInfo
{
private:
    virConnectPtr conn;
    void get_mem_info(long long *total_mem, long long *free_mem);
    void get_node_info(std::vector<std::map<std::string, std::vector<int> > > &nodes);
    int get_cpu_info(unsigned long &cpu_idle, unsigned long &cpu_total);
    int cpu_num;
public:
    NativeImplementSysInfo();
    std::string get_host_mem_usage();
    std::string get_host_cpu_usage();
    std::string get_host_node_info();
    ~NativeImplementSysInfo();
};

#endif // NATIVE_IMPLEMENT_SYSINFO_H

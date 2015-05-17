#ifndef SYSINFO_H
#define SYSINFO_H
#include <string>
class SysInfo
{
private:
    void get_mem_info(long long *total_mem, long long *free_mem);
public:
    SysInfo(){}
    std::string get_host_mem_usage();
    virtual ~SysInfo(){}
};
#endif

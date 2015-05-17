#include <string>
#include <sstream>
#include "sysinfo.h"

#define NUMA
#ifdef NUMA
#include <numa.h>
void SysInfo::get_mem_info(long long *total_mem, long long *free_mem){
    int nodenr=numa_max_node();
    long long total_mem_space=0;
    long long total_free_space=0;
    for(int i=0;i<=nodenr;i++)
    {
        long long node_free_space;
        long long node_total_space;
        node_total_space=numa_node_size64(i,&node_free_space);
        if(node_total_space==-1)
            continue;
        total_free_space+=node_free_space;
        total_mem_space+=node_total_space;
    }

    *total_mem = total_mem_space;
    *free_mem = total_free_space;
}
#else
void SysInfo::get_mem_info(long long *total_mem, long long *free_mem){
    *total_mem = 100;
    *free_mem = (*free_mem)*2;
}
#endif // NUMA


std::string SysInfo::get_host_mem_usage(){
    long long mem_total, mem_free;
    get_mem_info(&mem_total, &mem_free);
    std::stringstream ss;
    ss << "{";
    ss << "\"host_mem_total\": ";
    ss << mem_total;
    ss <<", \"host_mem_free\": ";
    ss << mem_free;
    ss <<"}";

    return ss.str();
}

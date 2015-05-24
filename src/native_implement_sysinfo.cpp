#include "native_implement_sysinfo.h"
#include <numa.h>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "globals.h"
#include "json.hpp"
using json = nlohmann::json;

NativeImplementSysInfo::NativeImplementSysInfo(){
    conn = virConnectOpen("qemu:///system");
    if(conn == NULL){
        LOG_ERROR("could not open libvirt connection");
        exit(-1);
    }
}

NativeImplementSysInfo::~NativeImplementSysInfo(){
    virConnectClose(conn);
}

void NativeImplementSysInfo::get_mem_info(long long *total_mem, long long *free_mem){
    json j;
    
    int nodenr=numa_max_node();
    long long total_mem_space=0;
    long long total_free_space=0;
    for(int i=0;i<=nodenr;i++)
    {
        long long node_free_space;
        long long node_total_space;
        node_total_space = numa_node_size64(i,&node_free_space);
        if(node_total_space==-1)
            continue;
        total_free_space+=node_free_space;
        total_mem_space+=node_total_space;
    }

    *total_mem = total_mem_space;
    *free_mem = total_free_space;
}

void NativeImplementSysInfo::get_node_info(std::vector<std::map<std::string, std::vector<int> > > &nodes){
    nodes.clear();
    
    int nodenr = numa_num_configured_nodes();
    DEBUG(nodenr);
    for(int i=0; i<nodenr; i++){
        std::map<std::string, std::vector<int> > m;
            
        long long node_free_space;
        long long node_total_space = numa_node_size64(i, &node_free_space);
        std::vector<int> v;
        v.push_back((int)(node_total_space/1024));
        v.push_back((int)(node_free_space/1024));
        m["mem"] = v;

        v.clear();
        struct bitmask *cpumask = numa_allocate_cpumask();
        if(numa_node_to_cpus(i, cpumask) < 0){
            LOG_ERROR("numa_node_to_cpus error");
            continue;
        }
        for(size_t i=0; i<cpumask->size; i++){
            if(numa_bitmask_isbitset(cpumask, i)){
                v.push_back(i);
            }
        }
        numa_free_cpumask(cpumask);
        m["cpu"] = v;

        nodes.push_back(m);
    }
}


std::string NativeImplementSysInfo::get_host_mem_usage(){
    long long mem_total, mem_free;
    get_mem_info(&mem_total, &mem_free);
    json j;
    j["host_mem_total"] = mem_total;
    j["host_mem_free"] = mem_free;
    return j.dump();
}


std::string NativeImplementSysInfo::get_host_node_info(){
    std::vector<std::map<std::string, std::vector<int> > > v;
    get_node_info(v);

    json j;
    char buf[64];
    j["node_num"] = v.size();
    j["nodes"] = json::object();
    for(size_t i=0; i < v.size(); i++){ // i: node id
        sprintf(buf, "%lu",  i);      // buf: string of node id
        j["nodes_id"].push_back(buf);
        std::map<std::string, std::vector<int> > m = v[i];
        std::vector<int> mem = m["mem"];
        std::vector<int> cpu = m["cpu"];
        j["nodes"][buf] = json::object();
        j["nodes"][buf]["node_mem_total"] = mem[0];
        j["nodes"][buf]["node_mem_free"] = mem[1];
        json cores;
        for(size_t j=0; j<cpu.size(); j++){
            cores.push_back(cpu[j]);
        }
        j["nodes"][buf]["cores"] = cores;
        
    }
    return j.dump();
}

std::string NativeImplementSysInfo::get_vm_info(){
    json j = json::array();
    int numDomains;
    int *activeDomains;
    numDomains = virConnectNumOfDomains(conn);
    DEBUG(numDomains);
    
    activeDomains = (int *)malloc(sizeof(int) * numDomains);
    numDomains = virConnectListDomains(conn, activeDomains, numDomains);
    for (int i = 0 ; i < numDomains ; i++) {
        virDomainPtr dom = virDomainLookupByID(conn, activeDomains[i]);
        virDomainInfo info;
        if(virDomainGetInfo(dom, &info) < 0){
            LOG_ERROR("could not get domain info");
            continue;
        }
        json j_dom;
        j_dom["id"] = activeDomains[i];
        DEBUG(info.maxMem);
        j_dom["mem_total"] = info.maxMem;
        j_dom["vcpu"] = info.nrVirtCpu;
        j_dom["name"] = virDomainGetName(dom);
        j.push_back(j_dom);
    }
    free(activeDomains);
    return j.dump();
}

int NativeImplementSysInfo::get_cpu_info(unsigned long &cpu_idle, unsigned long &cpu_total){
    std::ifstream ifs("/proc/stat");
    if(!ifs.is_open()){
        LOG_ERROR("could not find /proc/stat");
        return -1;
    }
    std::string cpu;
    unsigned long user, nice, sys, idle, iowait, irq, softirq;
    ifs >> cpu >> user >> nice >> sys >> idle
        >> iowait >> irq >> softirq;
    ifs.close();

    cpu_idle = idle;
    cpu_total = user + nice + sys + idle + iowait + irq + softirq;
    return 0;
}
std::string NativeImplementSysInfo::get_host_cpu_usage(){
    json j;
    unsigned long idle_time1, idle_time2, total_time1, total_time2;
    if(get_cpu_info(idle_time1, total_time1) < 0){
        j["status"] = "error";
        return j.dump();
    }
    DEBUG(idle_time1);
    DEBUG(total_time1);
    
    usleep(10000);               // sleep for 10ms

    if(get_cpu_info(idle_time2, total_time2) < 0){
        j["status"] = "error";
        return j.dump();
    }
    DEBUG(idle_time2);
    DEBUG(total_time2);
    
    j["cpus_usage"] = (1.0 - ((double)(idle_time2 - idle_time1)/(total_time2 - total_time1))) * 100.0;
    j["status"] = "ok";
    return j.dump();
}

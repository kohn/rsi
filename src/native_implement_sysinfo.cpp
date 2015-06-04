#include "native_implement_sysinfo.h"
#include <numa.h>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "tinyxml2.h"
#include "globals.h"
#include "json/json.h"

NativeImplementSysInfo::NativeImplementSysInfo(){
    conn = virConnectOpen("qemu:///system");
    if(conn == NULL){
        LOG_ERROR("could not open libvirt connection");
        exit(-1);
    }
    cpu_num = 0;
    std::ifstream ifs("/proc/cpuinfo");
    while(!ifs.eof()){
        std::string line;
        ifs >> line;
        if(line.find("processor") == std::string::npos){
            cpu_num ++;
        }
    }
    ifs.close();
}

NativeImplementSysInfo::~NativeImplementSysInfo(){
    virConnectClose(conn);
}

void NativeImplementSysInfo::get_mem_info(long long *total_mem, long long *free_mem){
    std::ifstream ifs("/proc/meminfo");
    if(!ifs.is_open()){
        LOG_ERROR("could not open /proc/meminfo");
        *total_mem = 0;
        *free_mem = 0;
        return;
    }
    std::string label;
    long long total, free;
    ifs >> label >> total >> label;
    ifs >> label >> free >> label;
    *total_mem = total;
    *free_mem = free;
    ifs.close();
}

void NativeImplementSysInfo::get_node_info(std::vector<std::map<std::string, std::vector<int> > > &nodes){
    nodes.clear();
    if(numa_available() == -1){ // host is not numa arch
        long long total_mem, free_mem;
        get_mem_info(&total_mem, &free_mem);
        std::map<std::string, std::vector<int> > m;
        
        std::vector<int> v;
        v.push_back((int)total_mem);
        v.push_back((int)free_mem);
        m["mem"] = v;

        v.clear();
        for(int i=0; i<cpu_num; i++){
            v.push_back(i);
        }
        m["cpu"] = v;
    }
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
    Json::Value j;
    j["host_mem_total"] = mem_total;
    j["host_mem_free"] = mem_free;
    return j.toStyledString();
}


std::string NativeImplementSysInfo::get_host_node_info(){
    std::vector<std::map<std::string, std::vector<int> > > v;
    get_node_info(v);

    Json::Value j(Json::objectValue);
    char buf[64];
    j["node_num"] = (int)v.size();
    Json::Value nodes(Json::objectValue);
    Json::Value nodes_id(Json::arrayValue);
    for(size_t i=0; i < v.size(); i++){ // i: node id
        sprintf(buf, "%lu",  i);      // buf: string of node id
        nodes_id.append(buf);
        std::map<std::string, std::vector<int> > m = v[i];
        std::vector<int> mem = m["mem"];
        std::vector<int> cpu = m["cpu"];
        Json::Value jnode(Json::objectValue);
        jnode["node_mem_total"] = mem[0];
        jnode["node_mem_free"] = mem[1];
        Json::Value cores(Json::arrayValue);
        for(size_t j=0; j<cpu.size(); j++){
            cores.append(cpu[j]);
        }
        jnode["cores"] = cores;
        nodes[buf] = jnode;
    }
    j["nodes"] = nodes;
    j["nodes_id"] = nodes_id;
    return j.toStyledString();
}

std::string NativeImplementSysInfo::get_vm_info(){
    Json::Value j(Json::arrayValue);
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
        Json::Value j_dom(Json::objectValue);
        j_dom["id"] = activeDomains[i];
        j_dom["mem_total"] = (unsigned long long)info.maxMem;
        j_dom["vcpu"] = info.nrVirtCpu;
        j_dom["name"] = virDomainGetName(dom);
        j.append(j_dom);
    }
    free(activeDomains);
    return j.toStyledString();
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
    Json::Value j;
    unsigned long idle_time1, idle_time2, total_time1, total_time2;
    if(get_cpu_info(idle_time1, total_time1) < 0){
        j["status"] = "error";
        return j.toStyledString();
    }
    DEBUG(idle_time1);
    DEBUG(total_time1);
    
    usleep(10000);               // sleep for 10ms

    if(get_cpu_info(idle_time2, total_time2) < 0){
        j["status"] = "error";
        return j.toStyledString();
    }
    DEBUG(idle_time2);
    DEBUG(total_time2);
    
    j["cpus_usage"] = (1.0 - ((double)(idle_time2 - idle_time1)/(total_time2 - total_time1))) * 100.0;
    j["status"] = "ok";
    return j.toStyledString();
}

std::string NativeImplementSysInfo::get_vm_detail(int domain_id){
    Json::Value j;
    j["status"] = "ok";
    virDomainPtr dom = virDomainLookupByID(conn, domain_id);
    virDomainInfo info;
    if(virDomainGetInfo(dom, &info) < 0){
        LOG_ERROR("could not get domain info");
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    // basic info
    j["id"] = domain_id;
    j["mem_total"] = (unsigned long long)info.maxMem;
    j["vcpu"] = info.nrVirtCpu;
    j["name"] = virDomainGetName(dom);

    // more detailed info
    char *domain_xml = virDomainGetXMLDesc(dom, VIR_DOMAIN_XML_SECURE);
    tinyxml2::XMLDocument doc;
    doc.Parse(domain_xml);
    j["img_path"] = doc.RootElement()->FirstChildElement("devices")->FirstChildElement("disk")->FirstChildElement("source")->Attribute("file");
    tinyxml2::XMLElement *graphics = doc.RootElement()->FirstChildElement("devices")->FirstChildElement("graphics");
    j["vnc_port"] = graphics->Attribute("port");
    
    return j.toStyledString();
}


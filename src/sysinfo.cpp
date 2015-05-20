#include <string>
#include <sstream>
#include "sysinfo.h"
#include "vsf.h"
#include "json.hpp"
#include <stdlib.h>
#include <iostream>
using json = nlohmann::json;

#ifdef LIBVSF
SysInfo::SysInfo(){
    framework = Vsf::get_instance();
    framework->init({
        { Option::OP_HS_NODE_CPU, { } },
        { Option::OP_HS_NODE_SYS_DIST, { } },
        { Option::OP_VM_BASE,
            {
                { OptionParam::VM_CMD, "qemu-system-x86_64" },
                { OptionParam::LOOP_INTERVAL, 3000 }//,
            }
        },
        { Option::OP_VM_CPU_USAGE,
            {
                { OptionParam::LOOP_INTERVAL, 3000 }//,
            }
        },
        { Option::OP_VM_CACHE_MISS,
            {
                { OptionParam::LOOP_INTERVAL, 2000 },
                { OptionParam::SAMPLE_INTERVAL, 50000 }//,
            }
        }
    });

    host = framework->init_host();

}

std::string SysInfo::get_host_node_info(){
    char buf[64];
    
    json j;
    j["node_num"] = host->node_num();

    j["nodes"] = json::object();
    std::set<NodeId> node_ids = host->node_ids();
    for(auto &id : node_ids){
        sprintf(buf, "%d",  id.id);
        j["nodes_id"].push_back(buf);
    }

    for(auto &id : node_ids){
        sprintf(buf, "%d",  id.id);
        j["nodes"][buf]["node_mem_total"] = 300000;
        j["nodes"][buf]["node_mem_free"] = 160000;
    }
    std::cout << j.dump() << std::endl;
    std::set<CoreId> core_ids = host->core_ids();
    for(auto &id : core_ids){
        sprintf(buf, "%d",  host->node_id(id).id);
        j["nodes"][buf]["cores"].push_back(id.core_id);
    }
    return j.dump();
}

std::string SysInfo::get_vm_info(){
    std::set<VM> vms = framework->init_vms(host);
    json j;

    for(auto &vm : vms){
        json obj = json::object();
        obj["id"] = vm.uuid();
        obj["name"] = vm.name();
        obj["mem_total"] = vm.total_mem_size();
        obj["vcpu"] = vm.vcpu_num();
        obj["cpu_usage"] = vm.cpu_usage();
        j.push_back(obj);
    }
    return j.dump();
}
#endif // LIBVSF



#include "virt.h"
#include "globals.h"
#include <stdlib.h>
#include "tinyxml2.h"
#include "globals.h"
#include "json/json.h"
#include <vector>
#include <fstream>
#include "tools.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rsi_client.h"
#include <cstdlib>
#include <unistd.h>
VM_Controller::VM_Controller(){
    _conn = virConnectOpen("qemu:///system");
    if(_conn == NULL){
        LOG_ERROR("could not open libvirt connection");
        exit(-1);
    }
}

VM_Controller::~VM_Controller(){
    virConnectClose(_conn);
}

int VM_Controller::create_vm(int vcpu, int mem, std::string img_path){
    
    return 0;
}

std::string VM_Controller::open_vm(std::string name){
    Json::Value j(Json::objectValue);
    j["status"] = "ok";
    
    virDomainPtr dom = virDomainLookupByName(_conn, name.c_str());
    if(dom == NULL){
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    if(virDomainCreate(dom) < 0){
        j["status"] = "could not open domain";
        return j.toStyledString();
    }

    int dom_id;
    if( (dom_id = virDomainGetID(dom)) < 0){
        j["status"] = "could not get domain id";
        return j.toStyledString();
    }
    j["vm_id"] = dom_id;
    return j.toStyledString();
}

std::string VM_Controller::get_vm_info(){
    Json::Value j(Json::arrayValue);
    int numActiveDomains = virConnectNumOfDomains(_conn);
    int *activeDomainsIDs = (int *)malloc(sizeof(int) * numActiveDomains);
    
    numActiveDomains = virConnectListDomains(_conn,
                                             activeDomainsIDs,
                                             numActiveDomains);
    for (int i = 0 ; i < numActiveDomains ; i++) {
        virDomainPtr dom = virDomainLookupByID(_conn, activeDomainsIDs[i]);
        virDomainInfo info;
        if(virDomainGetInfo(dom, &info) < 0){
            LOG_ERROR("could not get domain info");
            continue;
        }
        Json::Value j_dom(Json::objectValue);
        j_dom["id"] = activeDomainsIDs[i];
        j_dom["mem_total"] = (unsigned long long)info.maxMem;
        j_dom["vcpu"] = info.nrVirtCpu;
        j_dom["name"] = virDomainGetName(dom);
        j_dom["status"] = "running";
        j.append(j_dom);
    }
    free(activeDomainsIDs);

    int numInactiveDomains = virConnectNumOfDefinedDomains(_conn);
    char **inactiveDomainsNames = (char **)malloc(sizeof(char *)
                                                  * numInactiveDomains);
    numInactiveDomains = virConnectListDefinedDomains(_conn,
                                                      inactiveDomainsNames,
                                                      numInactiveDomains);
    if(numInactiveDomains < 0){
        LOG_ERROR("could not get defined domains");
        exit(-1);
    }
    
    for(int i=0; i< numInactiveDomains; i++){
        Json::Value j_dom(Json::objectValue);
        j_dom["status"] = "shutoff";
        j_dom["name"] = inactiveDomainsNames[i];
        free(inactiveDomainsNames[i]);
        j.append(j_dom);
    }
    free(inactiveDomainsNames);
    
    return j.toStyledString();
}

std::string VM_Controller::get_vm_detail_by_name(std::string domain_name){
    Json::Value j;
    virDomainPtr dom = virDomainLookupByName(_conn, domain_name.c_str());
    if(dom == NULL){
        Json::Value j;
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    std::string res = get_vm_detail(dom);
    virDomainFree(dom);
    return res;
}

std::string VM_Controller::get_vm_detail(virDomainPtr dom){
    Json::Value j;
    j["status"] = "ok";
    virDomainInfo info;
    if(virDomainGetInfo(dom, &info) < 0){
        LOG_ERROR("could not get domain info");
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    // basic info
    int state;
    virDomainGetState(dom, &state, NULL, 0);
    j["vm_status"] = state_code2string(state);
    if(virDomainIsActive(dom)){
        j["id"] = virDomainGetID(dom);
    }
    j["name"] = virDomainGetName(dom);
    j["vcpu"] = virDomainGetMaxVcpus(dom);
    j["mem_total"] = (unsigned long long)virDomainGetMaxMemory(dom);

    // more detailed info
    char *domain_xml = virDomainGetXMLDesc(dom, VIR_DOMAIN_XML_SECURE);
    tinyxml2::XMLDocument doc;
    doc.Parse(domain_xml);
    j["img_path"] = doc.RootElement()
        ->FirstChildElement("devices")
        ->FirstChildElement("disk")
        ->FirstChildElement("source")
        ->Attribute("file");
    tinyxml2::XMLElement *graphics = doc.RootElement()
        ->FirstChildElement("devices")
        ->FirstChildElement("graphics");
    j["vnc_port"] = graphics->Attribute("port");

    virDomainFree(dom);
    
    return j.toStyledString();
}
std::string VM_Controller::get_vm_detail_by_id(int domain_id){
    virDomainPtr dom = virDomainLookupByID(_conn, domain_id);
    if(dom == NULL){
        Json::Value j;
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    std::string res = get_vm_detail(dom);
    virDomainFree(dom);
    return res;
}

std::string VM_Controller::close_vm(int domain_id){
    Json::Value j(Json::objectValue);
    j["status"] = "ok";

    virDomainPtr dom = virDomainLookupByID(_conn, domain_id);
    if(dom == NULL){
        j["status"] = "no such domain";
        return j.toStyledString();
    }
    if(virDomainDestroy(dom) < 0){
        j["status"] = "could not shutdown domain";
        return j.toStyledString();
    }
    return j.toStyledString();
}


std::string VM_Controller::state_code2string(int state){
    switch(state){
    case VIR_DOMAIN_NOSTATE: return "nostate";
    case VIR_DOMAIN_RUNNING: return "running";
    case VIR_DOMAIN_BLOCKED: return "blocked";
    case VIR_DOMAIN_PAUSED: return "paused";
    case VIR_DOMAIN_SHUTDOWN: return "shutdown";
    case VIR_DOMAIN_SHUTOFF: return "shutoff";
    case VIR_DOMAIN_CRASHED: return "crashed";
    }
    return "unknown";
}

std::string VM_Controller::_get_vm_mac(virDomainPtr dom){
    char *domain_xml = virDomainGetXMLDesc(dom,
                                           VIR_DOMAIN_XML_SECURE);
    tinyxml2::XMLDocument doc;
    doc.Parse(domain_xml);
    std::string mac = doc.RootElement()
        ->FirstChildElement("devices")
        ->FirstChildElement("interface")
        ->FirstChildElement("mac")
        ->Attribute("address");
    return mac;
}


// read /var/lib/libvirt/dnsmasq/default.leases to get ip
// domain must use DHCP
std::string VM_Controller::get_vm_ip_by_name(std::string domain_name){
    LOG_INFO(domain_name);
    Json::Value j;
    virDomainPtr dom = virDomainLookupByName(_conn, domain_name.c_str());
    if(dom == NULL){
        Json::Value j;
        j["status"] = "no such domain";
        return j.toStyledString();
    }
        
    std::string mac = _get_vm_mac(dom);
    LOG_INFO(mac);
    virDomainFree(dom);

    std::ifstream leases("/var/lib/libvirt/dnsmasq/default.leases");
    while(!leases.eof()){
        std::string line;
        std::getline(leases, line);
        DEBUG(line);
        if(line == "")
            break;
        std::vector<std::string> v;
        Tools::split(line, v, ' ');
        if(v[1] == mac){
            j["ip"] = v[2];
            j["status"] = "ok";
            return j.toStyledString();
        }
    }
    leases.close();
    j["status"] = "Could not find ip for the mac address";
    return j.toStyledString();
}

std::string VM_Controller::port_forward(std::string host_ip_address,
                                        std::string ip_address,
                                        std::string port){
    std::vector<std::string> v;
    Tools::split(ip_address, v, '.');
    std::string host_port = v[3] + port;
    host_port = "15000";
    
    std::string cmd = "iptables -t nat -A PREROUTING -p tcp -d ";
    cmd += host_ip_address;
    cmd +=" --dport ";
    cmd += host_port;
    cmd += " -j DNAT --to ";
    cmd += ip_address + ":" + port;
    DEBUG(cmd);
    system(cmd.c_str());
    cmd = "iptables -I FORWARD -p tcp -d ";
    cmd += ip_address + "  --dport " + port + " -j ACCEPT";
    DEBUG(cmd);
    system(cmd.c_str());

    Json::Value j;
    j["status"] = "ok";
    j["host_port"] = host_port;
    return j.toStyledString();
}

int VM_Controller::get_image_fd_by_name(std::string vm_name){
    virDomainPtr dom = virDomainLookupByName(_conn, vm_name.c_str());
    if(dom == NULL){
        return -1;
    }
    std::string img_path = _get_vm_image_path(dom);
    virDomainFree(dom);

    int fd = open(img_path.c_str(), O_RDONLY);
    return fd;
}
std::string VM_Controller::_get_vm_image_path(virDomainPtr dom){
    char *domain_xml = virDomainGetXMLDesc(dom, VIR_DOMAIN_XML_SECURE);
    tinyxml2::XMLDocument doc;
    doc.Parse(domain_xml);
    std::string img_path = doc.RootElement()
        ->FirstChildElement("devices")
        ->FirstChildElement("disk")
        ->FirstChildElement("source")
        ->Attribute("file");
    return img_path;
}

long VM_Controller::_get_vm_image_size(virDomainPtr dom){
    std::string img_path = _get_vm_image_path(dom);
    struct stat file_stat;
    if(stat(img_path.c_str(), &file_stat) == 0){
        return file_stat.st_size;
    }
    else{
        perror("_get_file_size");
        return -1l;
    }
}

long VM_Controller::get_image_size_by_name_in_long(std::string vm_name){
    virDomainPtr dom = virDomainLookupByName(_conn, vm_name.c_str());
    if(dom == NULL){
        return -1l;
    }
    else
        return _get_vm_image_size(dom);
}

std::string VM_Controller::fetch_image_by_name(std::string host_ip,
                                               int rsi_server_port,
                                               std::string vm_name){
    Json::Value j;
    RSIClient rsi_client(host_ip, rsi_server_port);
    if(!rsi_client.good()){
        j["status"] = "could not connect to host";
        return j.toStyledString();
    }

    std::string msg = "GET_IMAGE_FD_BY_NAME ";
    msg += vm_name;
    std::string fd_and_size = rsi_client.communicate(msg);

    std::stringstream ss(fd_and_size);
    std::string fd;
    long filesize;
    ss >> fd;
    // check fd is valid
    if( fd == "-1"){
        j["status"] = "remote server could not open image file";
        return j.toStyledString();
    }
    ss >> filesize;

    // find a available file name for the new image
    int img_version = 0;
    std::string imgdir = "/opt/";
    while(1){
        std::stringstream tmpname;
        tmpname << imgdir << vm_name << img_version << ".img";
        int fd = open(tmpname.str().c_str(), O_RDONLY);
        if(fd == -1){
            close(fd);
            break;
        }
        else{
            img_version++;
            close(fd);
        }
    }
    std::stringstream tmpname;
    tmpname << imgdir << vm_name << img_version << ".img";
    std::string img_name = tmpname.str();
    LOG_INFO(img_name);

    // use rsi_client to download image from remote rsi_server
    msg = "GET_IMAGE_BY_FD ";
    msg += fd;
    if( rsi_client.download(msg, img_name, filesize) == 0){
        j["status"] = "ok";
        j["img_path"] = img_name;
        return j.toStyledString();
    }
    else{
        j["status"] = "download error";
        return j.toStyledString();
    }
}

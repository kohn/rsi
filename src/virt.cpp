#include "virt.h"
#include "globals.h"
#include <stdlib.h>
#include "tinyxml2.h"
#include "globals.h"
#include "json/json.h"
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

int VM_Controller::CreateVM(int vcpu, int mem, std::string img_path){
    
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
    
    numActiveDomains = virConnectListDomains(_conn, activeDomainsIDs, numActiveDomains);
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
    char **inactiveDomainsNames = (char **)malloc(sizeof(char *) * numInactiveDomains);
    numInactiveDomains = virConnectListDefinedDomains(_conn, inactiveDomainsNames, numInactiveDomains);
    if(numInactiveDomains < 0){
        LOG_ERROR("could not get defined domains");
        exit(-1);
    }
    
    for(int i=0; i< numInactiveDomains; i++){
        //virDomainPtr dom = virDomainLookupByName(inactiveDomainsNames[i]);
        Json::Value j_dom(Json::objectValue);
        j_dom["status"] = "shutoff";
        j_dom["name"] = inactiveDomainsNames[i];
        free(inactiveDomainsNames[i]);
        j.append(j_dom);
    }
    free(inactiveDomainsNames);
    
    return j.toStyledString();
}

std::string VM_Controller::get_vm_detail(int domain_id){
    Json::Value j;
    j["status"] = "ok";
    virDomainPtr dom = virDomainLookupByID(_conn, domain_id);
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


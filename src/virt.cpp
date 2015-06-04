#include "virt.h"
#include "globals.h"
#include <stdlib.h>
VM_Controller::VM_Controller(){
    conn = virConnectOpen("qemu:///system");
    if(conn == NULL){
        LOG_ERROR("could not open libvirt connection");
        exit(-1);
    }
}

VM_Controller::~VM_Controller(){
    virConnectClose(conn);
}

int VM_Controller::CreateVM(int vcpu, int mem, std::string img_path){
    
    return 0;
}

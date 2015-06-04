#ifndef VIRT_H
#define VIRT_H
#include <libvirt/libvirt.h>
#include <string>
class VM_Controller
{
private:
    virConnectPtr conn;
public:
    int CreateVM(int vcpu, int mem, std::string img_path);
    VM_Controller();
    ~VM_Controller();
};
#endif

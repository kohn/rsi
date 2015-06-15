#ifndef VIRT_H
#define VIRT_H
#include <libvirt/libvirt.h>
#include <string>
class VM_Controller
{
private:
    virConnectPtr _conn;
public:
    int CreateVM(int vcpu, int mem, std::string img_path);
    /* [
     *     {
     *         "id": id,
     *         "name": name,
     *         "mem_total": mem_total,
     *         "vcpu": vcpu,
     *         "cpu_usage": cpu_usage,
     *         "status": "running"
     *     }
     *     {
     *         "name": name,
     *         "status": "shutoff"
     *     }
     * ]
    */
    std::string get_vm_info();
    std::string get_vm_detail(int domain_id);
    std::string open_vm(std::string name);
    std::string close_vm(int domain_id);
    VM_Controller();
    ~VM_Controller();
};
#endif

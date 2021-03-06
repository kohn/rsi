#ifndef VIRT_H
#define VIRT_H
#include <libvirt/libvirt.h>
#include <string>
class VM_Controller
{
private:
    virConnectPtr _conn;
    std::string state_code2string(int state);
    std::string get_vm_detail(virDomainPtr dom);
    std::string _get_vm_mac(virDomainPtr dom);
    std::string _get_vm_image_path(virDomainPtr dom);
    std::string _make_tmp_config_file_name(std::string vm_name);
    long long _get_vm_image_size(virDomainPtr dom);

public:
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
    /*
     * {
     *     "status": "ok"|"no such domain",
     *     "id": id,
     *     "mem_total": mem_total_in_KB,
     *     "vcpu": vcpu_num,
     *     "name": name,
     *     "img_path": path_to_image,
     *     "vnc_port": VNC_port
     * }
     */
    std::string get_vm_detail_by_id(int domain_id);
    std::string get_vm_detail_by_name(std::string domain_name);
    /* {
     *     "status": "ok"|error_reason,
     *     "vm_id": vm_id if status==ok
     * }
    */
    std::string open_vm(std::string name);
    /* {
     *     "status": "ok"|error_reason
     * }
    */
    std::string close_vm(int domain_id);
    std::string get_vm_ip_by_name(std::string domain_name);
    // do port forwarding on the host
    std::string port_forward(std::string host_ip_address,
                             std::string ip_address,
                             std::string port);
    std::string fetch_image_by_name(std::string host_ip,
                                    int rsi_server_port,
                                    std::string vm_name,
                                    std::string new_name);
    long long get_image_size_by_name_in_ll(std::string vm_name);
    long long get_image_config_size_by_name_in_ll(std::string vm_name);
    int get_image_fd_by_name(std::string vm_name); // do remember to close the returned fd.
    int get_image_config_fd_by_name(std::string vm_name);
    std::string get_job_progress(int job_id);
    std::string new_vm_config(std::string vm_name, std::string img_path,
                              std::string conifg_path, int cpu_num, int mem_in_mb);
    VM_Controller();
    ~VM_Controller();
};
#endif

#ifndef SYSINFO_H
#define SYSINFO_H
#include <string>
class SysInfo
{
public:
    SysInfo(){}
    /* {
     *     "host_mem_total": mem_total,
     *     "host_mem_free": mem_free
     * }
    */
    virtual std::string get_host_mem_usage() = 0;
    /* {
     *     "node_num": node_num,
     *     "nodes_id": [id0, id1, ...],
     *     "nodes": {
     *                  id0: {
     *                           "node_mem_total": node_mem_total,
     *                           "node_mem_free": node_mem_used
     *                        }
     *              }
     * }
    */
    virtual std::string get_host_node_info() = 0;
    /* {
     *     "status": "ok"|"error",
     *     "cpu_usage": cpu_usage
     * }
    */
    virtual std::string get_host_cpu_usage() = 0;
    virtual ~SysInfo(){}
};
#endif

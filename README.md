RSI
===

RSI attempts to work as a server and responses to requests with system's
status including CPU usage, memory usage and virtual machine information
vis TCP sockets.

Dependencies
============

1.  CMake
2.  libvirt
3.  libproc
4.  libnuma
5.  gcc(g++) 4.9.0+

Interface
=========

Host Memory
-----------

Input: `GET_HOST_MEM_USAGE`

Output:

``` {.json}
  { 
       "host_mem_total": mem_total,
       "host_mem_free": mem_free
  }    
```

Host CPU
--------

Input: `GET_HOST_CPU_USAGE`

Output:

``` {.json}
  {
      "status": "ok"|"error",
      "cpu_usage": cpu_usage
  }
```

Host Nodes Information
----------------------

Input: `GET_HOST_NODE_INFO`

Output:

``` {.json}
  {
      "node_num": node_num,
      "nodes_id": [id0, id1, ...],
      "nodes": {
          id0: {
              "node_mem_total": node_mem_total,
              "node_mem_free": node_mem_used
          }
      }
  }
```

Host VM Information
-------------------

Input: `GET_VM_INFO`

Output:

``` {.json}
  [
      {
          "id": id,
          "name": name,
          "mem_total": mem_total,
          "vcpu": vcpu,
          "cpu_usage": cpu_usage,
      }
      { ... }
  ]
```

VM Detail
---------

Input: `GET_VM_INFO\nvm_id` Output:

``` {.json}
  {
      "status": "ok"|"no such domain",
      "id": id,
      "mem_total": mem_total_in_KB,
      "vcpu": vcpu_num,
      "name": name,
      "img_path": path_to_image,
      "vnc_port": VNC_port
  }
```

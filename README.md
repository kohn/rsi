RSI
===

RSI attempts to work as a server and responses to requests with system's
status including CPU usage, memory usage and virtual machine information
via TCP sockets.

RSI server works as daemon process and listens to port 7209(could be
changed in rsi.config). After
connected to a client, it reads message from client and sends
corresponding message in JSON format back to client.

Dependencies
============

1.  CMake 2.8+
2.  libvirt
3.  libproc
4.  libnuma
5.  gcc(g++) 4.9.0+ (-std=c++11)
6.  tinyxml2(included in this repo)

Interface
=========

Host Memory
-----------

Input: `GET_HOST_MEM_USAGE`

Output:

``` 
  { 
       "host_mem_total": mem_total,
       "host_mem_free": mem_free
  }    
```

Host CPU
--------

Input: `GET_HOST_CPU_USAGE`

Output:

``` 
  {
      "status": "ok"|"error",
      "cpu_usage": cpu_usage
  }
```

Host Nodes Information
----------------------

Input: `GET_HOST_NODE_INFO`

Output:

``` 
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

``` 
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

Input: `GET_VM_INFO vm_id` Output:

``` 
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

Unknown Input
------------
```
{
    "status": "cmd not recognized"
}
```

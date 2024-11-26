# RPC-DPDK-Client

This repository contains the implementation of a RPC client with DPDK networking stack. It generates requests based on an exponential Poisson inter-arrival distribution. We use it as the load generator for [DORADD](https://github.com/doradd-rt/doradd-server) experiments.

---

## Setup

**Network configuration - MAC address and Port ID**

Currently, before building the source code, one need to specify the MAC address and DPDK port ID. To find out the available DPDK port, you should use `dpdk/usertools/devbind.py -s` to check the port status and its DPDK port ID.

For instance, if the interface mac address is `1c:34:da:41:ca:bc`, given we manually set the client IP address is `<subnet>.3` (in [src/config.h](https://github.com/doradd-rt/rpc-dpdk-client/blob/97cbc69c3707b0c3f9be628d6452c4c1acbe24e2/src/config.h#L8)), you need to update the 3rd entry in [src/arp-config.h](https://github.com/doradd-rt/rpc-dpdk-client/blob/main/src/arp-cfg.h) as server mac address `1c:34:da:41:ca:bc` , based our pre-set L2 forwarding rule. And you also need to update the DPDK `PORT_ID` accordingly [here](https://github.com/doradd-rt/rpc-dpdk-client/blob/97cbc69c3707b0c3f9be628d6452c4c1acbe24e2/src/config.h#L11). Similarly, you need to update the client mac address accordingly in the arp entries. If the server IP is `<subnet>.2` , then you should update the 2nd entry with the server mac address.

## Build

```bash
git submodule update --init
make dpdk
cd scripts && sudo ./hugepages.sh
cd ../src && mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release 
ninja
```

## Run

1. Prepare input logs
    
    DORADD experiments takes a workload log as input, thus we need to pre-generate logs to avoid on-path generation overhead. The `scripts/gen-replay-log` includes the code to generate YCSB workload.
    
    ```bash
    pushd scripts/gen-replay-log
    g++ -O3 generate_ycsb_zipf.cc
    ./a.out -d uniform -c no_cont
    popd
    ```
    
2. Run
    
    ```bash
    sudo ./client -l 1-8 -- \ # cores
    -i 100 \ # inter-arrival
    -s ~/rpc-dpdk-client/scripts/gen-replay-log/ycsb_uniform_no_cont.txt \ # input logs
    -a ycsb \ # workload
    -t 192.168.1.2 \ # target destination IP
    -d 30 \ # duration
    -l /tmp/test.log # output log
    ```

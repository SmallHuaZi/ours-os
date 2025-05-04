# Experimental system topology
#
# +---------------------+              +---------------------+
# |     NUMA Node 0     |              |     NUMA Node 1     |
# |---------------------|+-----10-----+|---------------------|
# | CPU Cores: 0, 1     |              | CPU Cores: 2, 3     |
# | Memory Backend: m0  |              | Memory Backend: m1  |
# +---------------------+              +---------------------+
#          |                                      |
#          |                                      |
#         20                                     20
#          |                                      |
#          |                                      |
# +---------------------+               +---------------------+
# |     NUMA Node 2     |               |     NUMA Node 3     |
# |---------------------|+-----10------+|---------------------|
# | CPU Cores: 4, 5, 6  |               | CPU Core: 7         |
# | Memory Backend: m2  |               | Memory Backend: m3  |
# +---------------------+               +---------------------+
#
qemu-system-x86_64 -drive file=ours.iso,format=raw \
    -cpu max \
    -s -S   \
    -m 1G   \
    -smp 8  \
    -object memory-backend-ram,size=256M,id=m0 \
    -object memory-backend-ram,size=256M,id=m1 \
    -object memory-backend-ram,size=256M,id=m2 \
    -object memory-backend-ram,size=256M,id=m3 \
    -numa node,nodeid=0,cpus=0-1,memdev=m0 \
    -numa node,nodeid=1,cpus=2-3,memdev=m1 \
    -numa node,nodeid=2,cpus=4-6,memdev=m2 \
    -numa node,nodeid=3,cpus=7,memdev=m3 \
    -numa dist,src=0,dst=1,val=10 \
    -numa dist,src=1,dst=0,val=10 \
    -numa dist,src=0,dst=2,val=20 \
    -numa dist,src=2,dst=0,val=20 \
    -numa dist,src=1,dst=3,val=20 \
    -numa dist,src=3,dst=1,val=20 \
    -numa dist,src=2,dst=3,val=10 \
    -numa dist,src=3,dst=2,val=10 \
    -numa dist,src=0,dst=3,val=30 \
    -numa dist,src=3,dst=0,val=30 \
    -numa dist,src=1,dst=2,val=30 \
    -numa dist,src=2,dst=1,val=30 \
    -monitor telnet::45454,server,nowait \
    --nographic \
    -no-reboot \
    -d int,cpu_reset \
    -D file:qemu-log.txt \
    -serial mon:stdio
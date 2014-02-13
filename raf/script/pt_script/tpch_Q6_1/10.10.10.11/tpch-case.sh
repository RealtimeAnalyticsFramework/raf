#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# TPCH_HOME: working dir of tpch-gen

sysctl -w net.core.rmem_max=209630400
sysctl -w net.core.rmem_default=209630400
sysctl -w net.core.wmem_max=209630400
sysctl -w net.core.wmem_default=209630400
sysctl -w net.core.optmem_max=20480000
sysctl -w net.core.netdev_max_backlog=100000

cd /home/idgs

killall -9 idgs-aio 

echo "--------------- start idgs server 1 ------------------"
sleep 2
export idgs_member_port=7700
export idgs_member_innerPort=7701
export idgs_member_service_local_store=true
dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_tcp_srv.log &
tail -f idgs_tpch_tcp_srv.log


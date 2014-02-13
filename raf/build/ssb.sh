#!/bin/bash
#
# performance test for tpch
#
. build/common.sh

SSB_HOME="/tmp/ssb_pt"
export SSB_HOME
SSB_SIZE="0.1"
export SSB_SIZE

cd $WORKSPACE/idgs
. build/ssb-gen.sh
cd $WORKSPACE/idgs
. build/ssb-case.sh

GLOG_v=0
export GLOG_v

cd $WORKSPACE/idgs
make install-strip 1>/dev/null 2>&1

# OS tuning, su root
sysctl -w net.core.rmem_max=209630400
sysctl -w net.core.rmem_default=209630400
sysctl -w net.core.wmem_max=209630400
sysctl -w net.core.wmem_default=209630400
sysctl -w net.core.optmem_max=20480000
sysctl -w net.core.netdev_max_backlog=100000


killall -9 load
killall -9 idgs
killall -9 idgs-aio

run_tcpload() {
	tcpload
	#check_load
	#dist/itest/ssb_Q1_1
	safekill $IDGS_PID1
}

run_udpload() {
	udpload
	#check_load
	#dist/itest/ssb_Q1_1
	safekill $IDGS_PID1
}

run_tcpload_batchline() {
	tcpload_batchline
	#check_load
	dist/itest/ssb_Q1_1
	safekill $IDGS_PID1
}

run_all_load() {
	run_tcpload
	run_udpload
	run_tcpload_batchline
}

# default run all load
which_loader=-1;
if [ $# == 1 ]; then
	which_loader=$1;
fi

if [ $which_loader == 0 ]; then
	run_tcpload
elif [ $which_loader == 1 ]; then
	run_udpload
elif [ $which_loader == 2 ]; then
	run_tcpload_batchline
else
	run_all_load
fi

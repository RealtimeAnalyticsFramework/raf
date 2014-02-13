#!/bin/bash
#
# integration test for tpch
#
. build/common.sh

SSB_HOME="/tmp/ssb_it"
export SSB_HOME
SSB_SIZE="0.001"
export SSB_SIZE

cd $WORKSPACE/idgs
. build/ssb-gen.sh
cd $WORKSPACE/idgs
. build/ssb-case.sh

GLOG_v=0
export GLOG_v

killall -9 load
killall -9 idgs

run_tcpload() {
	tcpload
	#check_load
	dist/itest/ssb_Q1_1
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
	#dist/itest/ssb_Q1_1
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

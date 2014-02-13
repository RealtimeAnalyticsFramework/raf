#!/bin/bash
#
# integration test for tpch
#
. build/common.sh

TPCH_HOME="/tmp/tpch_it"
export TPCH_HOME
TPCH_SIZE="0.001"
export TPCH_SIZE

BATCH_INSERT_LINES="10"
export BATCH_INSERT_LINES


cd $WORKSPACE/idgs
. build/tpch-gen.sh
cd $WORKSPACE/idgs
. build/tpch-case.sh

GLOG_v=0
export GLOG_v

killall -9 load
killall -9 idgs
killall -9 idgs-aio

TPCH_Q6_LOOP=100
export TPCH_Q6_LOOP

run_tcpload() {
	tcpload
	#check_load
	#dist/itest/tpch_Q6
	safekill $IDGS_PID1
}

run_udpload() {
	udpload
	#check_load
	#dist/itest/tpch_Q6
	safekill $IDGS_PID1
}

run_tcpload_batchline() {
	tcpload_batchline
	#check_load
	#dist/itest/tpch_Q6
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

	

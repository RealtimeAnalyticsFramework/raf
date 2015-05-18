#!/bin/bash
#
# integration test for tpch
#
. build/common.sh

TPCH_SIZE="0.001"
export TPCH_SIZE
TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
export TPCH_HOME

BATCH_INSERT_LINES="10"
export BATCH_INSERT_LINES


cd $WORKSPACE/idgs
. build/tpch-gen.sh
cd $WORKSPACE/idgs
. build/tpch-case.sh

GLOG_v=0
export GLOG_v

# killall -9 load
# killall -9 idgs

TPCH_Q6_LOOP=100
export TPCH_Q6_LOOP



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

	

#!/bin/bash
#
# integration test for tpch
#
. build/common.sh

SSB_SIZE="0.001"
export SSB_SIZE
SSB_HOME="/tmp/ssb_$SSB_SIZE"
export SSB_HOME

cd $WORKSPACE/idgs
. build/ssb-gen.sh
cd $WORKSPACE/idgs
. build/ssb-case.sh


GLOG_v=0
export GLOG_v

# killall -9 load
# killall -9 idgs

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

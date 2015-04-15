#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# TPCH_HOME: working dir of tpch-gen

ensure_corosync(){
  echo "ensure corosync"
  # service corosync stop
  service corosync start
}

check_load(){
  store_names="Customer LineItem Nation Orders Part PartSupp Region Supplier"
  for store_name in $store_names
  do
  	export RDD_CHECK_STORE_NAME=$store_name
  	dist/itest/partition_count_action_it  
  done
}

cd $WORKSPACE/idgs

THREAD_COUNT=50;

start_servers() {
  # killall -9 idgs 
  ensure_corosync
  
  echo "start idgs server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs -c conf/cluster.conf 2>idgs_tpch_srv1.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 2 seconds"
  sleep 2
  echo "start idgs server 1"
  export idgs_member_port=7750
  export idgs_member_innerPort=7751
  export idgs_member_service_local_store=true
  dist/bin/idgs -c conf/cluster.conf 2>idgs_tpch_srv2.log &
  IDGS_PID2=$!
  export IDGS_PID2

  echo "wait for 4 seconds"
  sleep 4
}

tcpload() {
  start_servers
  dist/bin/idgs-load -s 0 -p $TPCH_HOME/dbgen -c conf/client.conf -m conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-tcptps.txt 1>load.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi

}

udpload() {
  start_servers

  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  export idgs_member_service_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-udptps.txt 1>load.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi

}

tcpload_batchline() {
  start_servers

  dist/bin/idgs-load -s 2 -p $TPCH_HOME/dbgen -c conf/client.conf -m conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-linecrudtps.txt 1>load.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
  
  sleep 2
}


# register EXIT trap
. build/archive_test_log.sh

run_tcpload() {
  echo "##################################################################"
  echo "     TPCH TCP"
  echo "##################################################################"
  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="TPCH-TCP"
  export IT_CASE_NAME

  tcpload
  #check_load 2>&1 | tee tpch_check.log
  export ACTION_TIMEOUT=60
  #dist/itest/tpch_Q6_1 2>&1 | tee tpch_q6.log
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi

  safekill $IDGS_PID1
  safekill $IDGS_PID2
}

run_udpload() {
  echo "##################################################################"
  echo "     TPCH UDP"
  echo "##################################################################"
  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="TPCH-UDP"
  export IT_CASE_NAME

  udpload
  #check_load 2>&1 | tee tpch_check.log
  export ACTION_TIMEOUT=60
  dist/itest/tpch_Q6_1 1>tpch_q6.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi

  safekill $IDGS_PID1
  safekill $IDGS_PID2
}

run_tcpload_batchline() {
  echo "##################################################################"
  echo "     TPCH Batch"
  echo "##################################################################"
  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="TPCH-BATCH"
  export IT_CASE_NAME

  tcpload_batchline
  #check_load 2>&1 | tee tpch_check.log
  export ACTION_TIMEOUT=60
  #dist/itest/tpch_Q6_1 2>&1 | tee tpch_q6.log
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi

  safekill $IDGS_PID1
  safekill $IDGS_PID2
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


#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# SSB_HOME: working dir of ssb-gen

ensure_corosync(){
  echo "ensure corosync"
  # service corosync stop
  service corosync start
}

check_load(){
  store_names="ssb_customer ssb_lineorder ssb_date ssb_part ssb_supplier"
  for store_name in $store_names
  do
  	export RDD_CHECK_STORE_NAME=$store_name
  	$BUILD_DIR/target/itest/partition_count_action_it  
  done
}

cd $WORKSPACE/idgs

THREAD_COUNT=150

start_servers() {
  ensure_corosync
  
  echo "start idgs server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  run_bg dist/bin/idgs -c conf/cluster.conf 2>idgs_ssb_srv1.log &
  IDGS_PID1=$!
  export IDGS_PID1

  echo "wait for 2 seconds"
  sleep 2
  echo "start idgs server 2"
  export idgs_public_port=7750
  export idgs_inner_port=7751
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf 2>idgs_ssb_srv2.log &
  IDGS_PID2=$!
  export IDGS_PID2

  
  echo "wait for 4 seconds"
  sleep 4
}

tcpload() {
  start_servers

  echo "standalone client"
  dist/bin/idgs-load -s 0 -p $SSB_HOME/ssb-dbgen-master -c conf/client.conf -m conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-tcptps.txt 1>load.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
}

udpload() {
  start_servers

  echo "in cluster client"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-udptps.txt 1>load.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
}

tcpload_batchline() {
  start_servers

  echo "linecrud standalone client"
  dist/bin/idgs-load -s 2 -p $SSB_HOME/ssb-dbgen-master -c conf/client.conf -m conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-linecrudtps.txt 1>load.log 2>&1
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
  echo "     SSB Batch"
  echo "##################################################################"

  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="SSB-TCP"
  export IT_CASE_NAME

  tcpload
  #check_load
  $BUILD_DIR/target/itest/ssb_Q1_1 1>ssbq1.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
  safekill $IDGS_PID1
  safekill $IDGS_PID2
}

run_udpload() {
  echo "##################################################################"
  echo "     SSB UDP"
  echo "##################################################################"
  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="SSB-UDP"
  export IT_CASE_NAME

  udpload
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
  safekill $IDGS_PID1
  safekill $IDGS_PID2
}

run_tcpload_batchline() {
  echo "##################################################################"
  echo "     SSB Batch"
  echo "##################################################################"
  rm *.log 2>/dev/null
  rm core* 2>/dev/null
  IT_CASE_NAME="SSB-BATCH"
  export IT_CASE_NAME
  tcpload_batchline
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





#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# SSB_HOME: working dir of ssb-gen

ensure_corosync(){
  # echo "restart corosync"
  service corosync stop
  service corosync start
}

check_load(){
  store_names="ssb_customer \
  			ssb_lineorder \
  			ssb_date \
  			ssb_part \
  			ssb_supplier"
  for store_name in $store_names
  do
  	export RDD_CHECK_STORE_NAME=$store_name
  	dist/itest/partition_count_action_it  
  done
}

cd $WORKSPACE/idgs

THREAD_COUNT=50;

tcpload() {
  echo "start corosync..."
  ensure_corosync
  
  echo "start idgs..."
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  run_bg dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_ssb_tcp.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 4 seconds"
  sleep 4

  echo "standalone client"
  time dist/bin/load -s 0 -p $SSB_HOME/ssb-dbgen-master -c samples/load/conf/client.conf -m samples/load/conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-tcptps.txt
}

udpload() {
  echo "start corosync..."
  ensure_corosync

  echo "start idgs..."
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  run_bg dist/bin/idgs-aio -c framework/conf/cluster.conf  2>idgs_ssb_udp.log &
  IDGS_PID1=$!
  export IDGS_PID1

  echo "wait for 4 seconds"
  sleep 4

  echo "in cluster client"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  export idgs_member_service_local_store=false
  time dist/bin/load -s 1 -p $SSB_HOME/ssb-dbgen-master -c framework/conf/cluster.conf -m samples/load/conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-udptps.txt
}

tcpload_batchline() {
  echo "start corosync..."
  ensure_corosync
  
  echo "start idgs..."
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  run_bg dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_ssb_tcp_linecrud.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 4 seconds"
  sleep 4

  echo "linecrud standalone client"
  time dist/bin/load -s 2 -p $SSB_HOME/ssb-dbgen-master -c samples/load/conf/client.conf -m samples/load/conf/ssb_file_mapper.conf -t $THREAD_COUNT -o ssb-linecrudtps.txt

}



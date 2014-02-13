#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# TPCH_HOME: working dir of tpch-gen

ensure_corosync(){
  # echo "restart corosync"
  service corosync stop
  service corosync start
}

check_load(){
  store_names="Customer \
  			LineItem \
  			Nation \
  			Orders \
  			Part \
  			PartSupp \
  			Region \
  			Supplier"
  for store_name in $store_names
  do
  	export RDD_CHECK_STORE_NAME=$store_name
  	dist/itest/partition_count_action_it  
  done
}

cd $WORKSPACE/idgs

THREAD_COUNT=50;

tcpload() {
  killall -9 idgs-aio 
  echo "restart corosync."
  ensure_corosync
  
  echo "start idgs server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_tcp_srv1.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 2 seconds"
  sleep 2
  echo "start idgs server 1"
  export idgs_member_port=7750
  export idgs_member_innerPort=7751
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_tcp_srv2.log &
  IDGS_PID2=$!
  export IDGS_PID2

  echo "wait for 4 seconds"
  sleep 4

  time dist/bin/load -s 0 -p $TPCH_HOME/dbgen -c samples/load/conf/client.conf -m samples/load/conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-tcptps.txt

}

udpload() {
  killall -9 idgs-aio 
  echo "restart corosync."
  ensure_corosync
  
  echo "start idgs server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_udp_srv1.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 2 seconds"
  sleep 2
  echo "start idgs server 1"
  export idgs_member_port=7750
  export idgs_member_innerPort=7751
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_udp_srv2.log &
  IDGS_PID2=$!
  export IDGS_PID2

  echo "wait for 4 seconds"
  sleep 4

  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  export idgs_member_service_local_store=false
  time dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-udptps.txt
  
}

tcpload_batchline() {
  killall -9 idgs-aio 
  echo "restart corosync."
  ensure_corosync
  
  echo "start idgs server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_batch_srv1.log &
  IDGS_PID1=$!
  export IDGS_PID1
  
  echo "wait for 2 seconds"
  sleep 2
  echo "start idgs server 1"
  export idgs_member_port=7750
  export idgs_member_innerPort=7751
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf 2>idgs_tpch_batch_srv2.log &
  IDGS_PID2=$!
  export IDGS_PID2
  
  echo "wait for 4 seconds"
  sleep 4

  time dist/bin/load -s 2 -p $TPCH_HOME/dbgen -c samples/load/conf/client.conf -m samples/load/conf/tpch_file_mapper.conf -t $THREAD_COUNT -o tpch-linecrudtps.txt

}


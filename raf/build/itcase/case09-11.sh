#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#


case9() {
  echo "########################"
  echo "Case 9: Store with actor to insert and get data test"
  echo " 1. start 1 server"
  echo " 2. insert and get data."
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core" -exec rm -f {} \; 2>/dev/null

  echo "start 1 server"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case9_server.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "start 1 client"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  run_test dist/itest/it_store_test_client_actor 

  echo "kill servers."
  safekill $SRV_PID1 
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case10() {
  echo "########################"
  echo "Case 10: Partition store insert and get test"
  echo " 1. start 1 server"
  echo " 2. start 2 server"
  echo " 3. start 1 client to insert data with partition store to server."
  echo " 4. start another client to get data."
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core" -exec rm -f {} \; 2>/dev/null

  echo "start 1 server"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case10_server1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "start 2 server"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case10_server2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  echo "start 1 client to insert data to server."
  run_test dist/itest/it_partition_store_test_insert 
  sleep 2

  echo "start 2 client to get data from server."
  run_test dist/itest/it_partition_store_test_get  
  sleep 2

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2
 
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case11() {
  echo "########################"
  echo "Case 11: Replicated store insert and get test"
  echo " 1. start 1 server"
  echo " 2. start 2 server"
  echo " 3. start 1 client to insert data with replicated store to server."
  echo " 4. start another client to get data."
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core" -exec rm -f {} \; 2>/dev/null

  echo "start 1 server"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case11_server1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "start 2 server"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case11_server2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  echo "start 1 client to insert data to server."
  run_test dist/itest/it_replicated_store_test_insert 
  sleep 2

  echo "start 2 client to get data from server."
  run_test dist/itest/it_replicated_store_test_get 
  sleep 2

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}


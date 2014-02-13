#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#


case14_base() {
  if [ "$1" -eq 1 ] ; then
    KILL="kill -9"
  else
    KILL=safekill
  fi
  
  cd $WORKSPACE/idgs/
  
  echo "step 1: start server 1(local store) "
  STEP=1
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server1_1.log 2>&1 &
  SRV_PID1=$!
  echo "sleep 2s..."
  sleep 2
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_client_1.log

  echo "step 2: start server 2(local store) "
  STEP=2
  export STEP
  export idgs_member_service_local_store=true
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server2_2.log 2>&1 &
  SRV_PID2=$!
  echo "sleep 2s..."
  sleep 2
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_client_2.log

  echo "step 3: start server 3(not local store) "
  STEP=3
  export STEP
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server3_3.log 2>&1 &
  SRV_PID3=$!
  echo "sleep 2s..."
  sleep 2
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  run_test dist/itest/it_cluster
  mv ut.log case14_client_3.log

  echo "step 4: kill server 1(local store, leading) "
  STEP=4
  export STEP
  $KILL $SRV_PID1
  run_test dist/itest/it_cluster
  mv ut.log case14_client_4.log
  
  echo "step 5: kill server 2(local store, leading) "
  STEP=5
  export STEP
  $KILL $SRV_PID2
  run_test dist/itest/it_cluster
  mv ut.log case14_client_5.log
  
  echo "step 6: start server 1, 2(local store) at the same time"
  STEP=6
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server6_1.log 2>&1 &
  SRV_PID1=$!
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server6_2.log 2>&1 &
  SRV_PID2=$!
  echo "sleep 2s..."
  sleep 2
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_client_6.log

  echo "step 7: kill server 1, 2(local store, without leading) at the same time"
  STEP=7
  export STEP
  $KILL $SRV_PID1 $SRV_PID2
  run_test dist/itest/it_cluster
  mv ut.log case14_client_7.log
  
  echo "step 8: start server 1, 2(local store) at the same time"
  STEP=8
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server8_1.log 2>&1 &
  SRV_PID1=$!
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case14_server8_2.log 2>&1 &
  SRV_PID2=$!
  echo "sleep 2s..."
  sleep 2
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_client_8.log

  echo "step 9: kill server 3(not local store, leading), 1(local store, possible selected leading) at the same time"
  STEP=9
  export STEP
  $KILL $SRV_PID3 $SRV_PID1
  run_test dist/itest/it_cluster
  mv ut.log case14_client_9.log

  echo "step 10: kill server 2(local store, leading) "
  STEP=10
  export STEP
  $KILL $SRV_PID2
  run_test dist/itest/it_cluster
  mv ut.log case14_client_10.log
  
  echo "checking coredump..." 
  check_core_dump dist/bin/idgs-aio
  echo "success!"
}


case14() {
  echo "##################################################################################"
  echo "Case 14 cluster integration test: kill "
  echo "##################################################################################"
  
  case14_base 0 
}

case15() {
  echo "##################################################################################"
  echo "Case 15 cluster integration test: kill -9"
  echo "##################################################################################"
  
  case14_base 1 
}

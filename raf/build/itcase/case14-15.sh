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
  
  rm -rf case14_*.log
  
  # sleep time seconds
  sleep_time=1
  
  step_desc="##############step 1: start server 0(local store), server 0 is leading. "
  STEP=1
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step1_server0.log 2>&1 &
  SRV_PID1=$!
  
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 2: start server 1(local store), server 0 is leading."
  STEP=2
  export STEP
  export idgs_member_service_local_store=true
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step2_server1.log 2>&1 &
  SRV_PID2=$!
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 3: kill server 1(local store), server 0 is leading."
  STEP=3
  export STEP
  echo $step_desc 
  echo $step_desc >>case14.log
  $KILL $SRV_PID2
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 4: start server 1(local store) again, server 0 is leading."
  STEP=4
  export STEP
  export idgs_member_service_local_store=true
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step4_server1.log 2>&1 &
  SRV_PID2=$!
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log
  
  step_desc="##############step 5: kill server 0(local store, leading), server 1 selected as new leading."
  STEP=5
  export STEP
  echo $step_desc 
  echo $step_desc >>case14.log
  $KILL $SRV_PID1
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log
  
  step_desc="##############step 6: start server 0(local store), server 2(not local store) at the same time, server 1 is leading."
  STEP=6
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step6_server0.log 2>&1 &
  SRV_PID1=$!
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/idgs -c conf/cluster.conf  1>case14_step6_server2.log 2>&1 &
  SRV_PID3=$!
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 7: kill server 0(local store), server 2(not local store) at the same time, server 1 is leading."
  STEP=7
  export STEP
  echo $step_desc 
  echo $step_desc >>case14.log
  $KILL $SRV_PID1 $SRV_PID3
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 8: start server 0(local store), server 2(not local store) at the same time again, server 1 is leading."
  STEP=8
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step8_server0.log 2>&1 &
  SRV_PID1=$!
  
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/idgs -c conf/cluster.conf  1>case14_step8_server2.log 2>&1 &
  SRV_PID3=$!
  
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 9: kill server 0(local store), server 1(local store, leading) at the same time, server 2 selected as new leading."
  STEP=9
  export STEP
  echo $step_desc 
  echo $step_desc >>case14.log
  $KILL $SRV_PID1 $SRV_PID2
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log
  
  step_desc="##############step 10: start server 0(local store), server 1(local store) at the same time again, server 2 is leading."
  STEP=10
  export STEP
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  echo $step_desc 
  echo $step_desc >>case14.log
  dist/bin/idgs -c conf/cluster.conf  1>case14_step10_server0.log 2>&1 &
  SRV_PID1=$!
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  export idgs_member_service_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case14_step10_server1.log 2>&1 &
  SRV_PID2=$!
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log

  step_desc="##############step 11: kill server 0, 1, 2 at the same time, test client itself is leading."
  STEP=11
  export STEP
  echo $step_desc 
  echo $step_desc >>case14.log
  $KILL $SRV_PID1 $SRV_PID2 $SRV_PID3
  echo "sleep $sleep_time s..."
  sleep $sleep_time 
  export idgs_member_port=30000
  export idgs_member_innerPort=30001
  export idgs_member_service_local_store=false
  run_test dist/itest/it_cluster
  mv ut.log case14_step${STEP}_client.log
  
  echo "checking coredump..." 
  check_core_dump dist/bin/idgs
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

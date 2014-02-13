#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#


case0() {
  echo "########################"
  echo "Case 0: start and stop 1 server"
  echo "########################"
  cd $WORKSPACE/idgs/
  echo "start server"
  dist/bin/idgs-aio 1>case0.log 2>&1 &
  IDGS_PID=$!
 
  echo "sleep 5 seconds" 
  sleep 5
  
  echo "kill server"
  safekill $IDGS_PID 

  check_core_dump dist/bin/idgs-aio
  #echo "########################"
}

case1() {
  echo "########################"
  echo "Case 1: start and stop servers"
  echo " 1. start 3 servers with interval 1s "
  echo " 2. stop the servers with interval 1s"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case1_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3
  echo "start server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf 1>case1_2.log 2>&1  &
  SRV_PID2=$!
  sleep 3
  echo "start server 3"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  dist/bin/idgs-aio -c framework/conf/cluster.conf 1>case1_3.log 2>&1  &
  SRV_PID3=$!
  
  sleep 4
  
  echo "kill server 1"
  safekill $SRV_PID1 
  sleep 1
  echo "kill server 2"
  safekill $SRV_PID2 
  sleep 1
  echo "kill server 3"
  safekill $SRV_PID3 
  
  check_core_dump dist/bin/idgs-aio
  #echo "########################"
}

case2() {
  echo "########################"
  echo "Case 2: start and stop servers"
  echo " 1. start 3 servers at the same time"
  echo " 2. stop the servers at the same time"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start all 3 servers."
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case2_1.log 2>&1 &
  SRV_PID1=$!
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf 1>case2_2.log 2>&1  &
  SRV_PID2=$!
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  dist/bin/idgs-aio -c framework/conf/cluster.conf 1>case2_3.log 2>&1  &
  SRV_PID3=$!
  
  echo "sleep 8s"
  sleep 8
  
  echo "kill all 3 servers."
  safekill $SRV_PID1 $SRV_PID2 $SRV_PID3
  
  check_core_dump dist/bin/idgs-aio
  #echo "########################"
}

case3() {
  echo "########################"
  echo "Case 3: Partition rebalancer"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  run_test dist/utest/partitiontable_balance_verifier_test 
  
  check_core_dump dist/utest/ut_manual
  #echo "########################"
}


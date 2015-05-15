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
  $VALGRIND dist/bin/idgs 1>case0.log 2>&1 &
  IDGS_PID=$!
 
  echo "sleep 5 seconds" 
  sleep 5
  
  echo "kill server"
  safekill $IDGS_PID 

  definitely_lost=`grep "definitely lost:" case0.log | sed -e 's/==[0-9]*==[a-z ]*: [0-9,]* bytes in \([0-9,]*\) blocks/\1/' -e 's/,//'`
  indirectly_lost=`grep "indirectly lost:" case0.log | sed -e 's/==[0-9]*==[a-z ]*: [0-9,]* bytes in \([0-9,]*\) blocks/\1/' -e 's/,//'`
  possibly_lost=`grep "possibly lost:" case0.log | sed -e 's/==[0-9]*==[a-z ]*: [0-9,]* bytes in \([0-9,]*\) blocks/\1/' -e 's/,//'`
  still_reachable=`grep "still reachable:" case0.log | sed -e 's/==[0-9]*==[a-z ]*: [0-9,]* bytes in \([0-9,]*\) blocks/\1/' -e 's/,//'`
  suppressed=`grep " suppressed:" case0.log | grep "==" | sed -e 's/==[0-9]*==[a-z ]*: [0-9,]* bytes in \([0-9,]*\) blocks/\1/' -e 's/,//'`

  cat <<END | tee case0_valgrind.txt
definitely_lost, indirectly_lost, possibly_lost, still_reachable, suppressed
$definitely_lost, $indirectly_lost, $possibly_lost, $still_reachable, $suppressed

END

  grep "==" case0.log 

  check_core_dump dist/bin/idgs
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
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case1_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3
  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf 1>case1_2.log 2>&1  &
  SRV_PID2=$!
  sleep 3
  echo "start server 3"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  dist/bin/idgs -c conf/cluster.conf 1>case1_3.log 2>&1  &
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
  
  check_core_dump dist/bin/idgs
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
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case2_1.log 2>&1 &
  SRV_PID1=$!
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf 1>case2_2.log 2>&1  &
  SRV_PID2=$!
  export idgs_public_port=9900
  export idgs_inner_port=9901
  dist/bin/idgs -c conf/cluster.conf 1>case2_3.log 2>&1  &
  SRV_PID3=$!
  
  echo "sleep 8s"
  sleep 8
  
  echo "kill all 3 servers."
  safekill $SRV_PID1 $SRV_PID2 $SRV_PID3
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case3() {
  echo "########################"
  echo "Case 3: Partition rebalancer"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  run_test $BUILD_DIR/target/utest/partitiontable_balance_verifier_test 
  
  check_core_dump $BUILD_DIR/target/utest/ut_manual
  #echo "########################"
}


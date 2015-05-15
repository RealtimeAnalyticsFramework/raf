#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case4() {
  echo "########################"
  echo "Case 4: TCP Udp Server Test"
  echo " 1. start server 1"
  echo " 2. start server 2"
  echo " 3. start client"
  echo " 4. client send one message to server 1 via TCP"
  echo " 5. server 1 send routeMessage to the server 2"
  echo " 6. the server 2 send response to the client"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  $BUILD_DIR/target/itest/it_rpc_tcp_test_server  1>case4_server.log 2>&1 &
  SRV_PID1=$!
  
  sleep 3

  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  $BUILD_DIR/target/itest/it_rpc_tcp_test_server2  1>case4_server2.log 2>&1 &
  SRV_PID2=$!
  
  sleep 3
  
  GLOG_v=10 run_test $BUILD_DIR/target/itest/it_rpc_tcp_test_client 
  
  sleep 2
  
  echo "kill servers."
  safekill $SRV_PID1 
  safekill $SRV_PID2 
 
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case5() {
  echo "########################"
  echo "Case 5: Dump actor descriptors"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  run_test $BUILD_DIR/target/utest/actor_descriptor_dump_test 
  
  #echo "########################"
}

case6() {
  echo "########################"
  echo "Case 6: load data via script(1 server, 1 client )"
  echo " 1. start 1 servers at the same time"
  echo " 2. load data"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start 1 server."
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case6_1.log 2>&1 &
  SRV_PID1=$!
  
  sleep 4

  echo "start 1 client."
  dist/bin/idgs-cli -f samples/shell/script/partition_insert 1>case6_client.log 2>&1
  
  sleep 2
  echo "kill all servers."
  safekill $SRV_PID1 
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case7() {
  echo "########################"
  echo "Case 7: load data via script(2 server, 1 client )"
  echo " 1. start 1 servers at the same time"
  echo " 2. load data"
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start server 1."
  dist/bin/idgs -c conf/cluster.conf   1>case7_1.log 2>&1 &
  SRV_PID1=$!
  echo "start server 2."
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case7_2.log 2>&1 &
  SRV_PID2=$!
  
  sleep 4

  echo "start 1 client."
  timeout 5s dist/bin/idgs-cli -f samples/shell/script/partition_insert 1>case7_client.log 2>&1
  
  sleep 2
  echo "kill all servers."
  safekill $SRV_PID1 
  safekill $SRV_PID2 
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case8() {
  echo "########################"
  echo "Case 8: TCP Server Test"
  echo " 1. start 1 server"
  echo " 2. start 1 client"
  echo " 3. start the other  client"
  echo " 4. client one and client two send the messages to server via TCP"
  echo " 5. the server send response to the clients"
  echo "########################"
 cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start 1 server"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  $BUILD_DIR/target/itest/it_rpc_tcp_test_server2  1>case8_server.log 2>&1 &
  SRV_PID1=$!
  sleep 2
  echo "start 1 client"
  $BUILD_DIR/target/itest/it_rpc_tcp_test_client2  1>case8_client.log 2>&1 &
  echo "start 2 client"
  run_test $BUILD_DIR/target/itest/it_rpc_tcp_test_client2
  
  sleep 2
  
  echo "kill servers."
  safekill $SRV_PID1 
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}


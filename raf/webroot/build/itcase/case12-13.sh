#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#


case12() {
  echo "########################"
  echo "Case 12: Replicated store syncronaize data test"
  echo " 1. start 1 server"
  echo " 2. start 1 client to insert data with replicated store to server."
  echo " 3. start 2 server"
  echo " 4. start another client to get data."
  echo "########################"
  cd $WORKSPACE/idgs/
  find . -name "core" -exec rm -f {} \; 2>/dev/null

  echo "start 1 server"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case12_server1.log 2>&1 &
  SRV_PID1=$!
  sleep 3

  echo "start 1 client to insert data to server."
  run_test $BUILD_DIR/target/itest/it_replicated_store_test_insert 

  echo "start 2 server"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case12_server2.log 2>&1 &
  SRV_PID2=$!
  sleep 3

  echo "start 2 client to get data from server."
  run_test $BUILD_DIR/target/itest/it_replicated_store_test_get 

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2
  
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case13() {
  echo "########################"
  echo "Case 13: scheduler Test"
  echo " 1. start 1 server"
  echo " 2. start 1 client, and send start work message to the server"
  echo " 3. the server send start work successfully response to the client, and then start the waiting client message timer"
  echo " 4. the client receive the message of server, but the client do not send the new message to server, so the timer of server triggered"
  echo " 5. the timer of server triggered, so it send the response send successfully to client, and start the timer"
  echo " 6. the client receive the message and then send new message comes to server, the test finish"
  echo " 7. the server send the number of times of the execution to client, just for check the result"
  echo "########################"
 cd $WORKSPACE/idgs/
  find . -name "core.*" -exec rm -f {} \; 2>/dev/null
  
  echo "start 1 server"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  $BUILD_DIR/target/itest/it_rpc_schedule_test_server 1>case13_server.log 2>&1 &
  SRV_PID1=$!
  sleep 2
  echo "start 1 client"
  run_test $BUILD_DIR/target/itest/it_rpc_schedule_test_client 
  
  echo "kill servers."
  safekill $SRV_PID1 
  
  check_core_dump $BUILD_DIR/target/itest/it_rpc_schedule_test_server
  #echo "########################"
}


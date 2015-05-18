#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case28() {
  echo "########################"
  echo "Case 28: Hash join test"
  echo " 1. start 2 servers"
  echo " 2. load ssb data, size 0.001G"
  echo " 3. test hash join, include inner join, left join, outer join"
  echo " 4. run test use ssb Q1.1"
  echo "    select sum(lo_extendedprice * lo_discount)"
  echo "      from lineorder, date"
  echo "     where lo_orderdate = d_datekey"
  echo "       and d_year = 1992"
  echo "       and lo_discount between 1 and 3"
  echo "       and lo_quantity < 25"
  echo "    execute sql result 100 times"
  echo "########################"
  cd $WORKSPACE/idgs/
  export GLOG_v=0
  

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case28_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case28_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  SSB_Q1_LOOP=1000
  export SSB_Q1_LOOP

  SSB_SIZE="0.001"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

  echo "generate ssb data"
  build/ssb-gen.sh

  rm -f tpch-*.txt  
  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m conf/ssb_file_mapper.conf -t 100 -o ssb-udptps.txt 1>it_case28.log 2>&1

  echo "run hash join test"  
  dist/bin/idgs-cli -f integration_test/rdd_it/hash_join_data 1>it_case28.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case28.log.";
    exit $RC
  fi

  $BUILD_DIR/target/itest/it_hash_join_transformer_test 1>>it_case28.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case28.log.";
    exit $RC
  fi
  
  echo "run SSB Q1.1 test"
  $BUILD_DIR/target/itest/ssb_q1_1_join_transformer 1>ut_result.log 2>>it_case28.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case28.log.";
    exit $RC
  fi

  rm -f tpch-*.txt

  echo ""
  echo "============ RESULT ============"
  cat ut_result.log
  echo "================================"

  echo "killing server 1"
  #safekill $SRV_PID1 
  #safekill $SRV_PID2
  
  # ensure kill all server
  #check_core_dump dist/bin/idgs
  #echo "########################"
}

case29() {
  echo "###############"	
  echo "Case 29: lookup action test"
  echo "###############"	

  export GLOG_v=0
  
  echo "start server 1"
  export idgs_local_store=true
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case29_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3

  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case29_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3

  SSB_SIZE="0.001"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

  echo "generate ssb data"
  build/ssb-gen.sh

  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m integration_test/rdd_it/lineorder_mapper.conf -t 100 1>it_case29.log 2>&1

  sleep 5
  
  echo "run case... details refer to it_case29.log."
  $BUILD_DIR/target/itest/it_lookup_action_test 1>>it_case29.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC).";
    exit $RC
  fi
  
  echo "kill servers..."
  safekill $SRV_PID1
  safekill $SRV_PID2
  check_core_dump dist/bin/idgs
}

case30() {
  echo "###############"	
  echo "Case 30: collect action test"
  echo "###############"	
  export GLOG_v=0
  
  echo "start server 1"
  export idgs_local_store=true
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case30_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3

  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case30_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3

  SSB_SIZE="0.001"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

  echo "generate ssb data"
  build/ssb-gen.sh


  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m integration_test/rdd_it/lineorder_mapper.conf -t 100 1>it_case30.log 2>&1

  sleep 5
  
  echo "run case... details refer to it_case30.log."
  $BUILD_DIR/target/itest/it_collect_action_test 1>>it_case30.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC).";
    exit $RC
  fi
  
  echo "kill servers..."
  safekill $SRV_PID1
  safekill $SRV_PID2
  check_core_dump dist/bin/idgs
}

case31() {
  echo "###############"	
  echo "Case 31: collect action test big data"
  echo "###############"	
  export GLOG_v=0
  
  echo "start server 1"
  export idgs_local_store=true
  export idgs_public_port=7700
  export idgs_inner_port=7701
  GLOG_vmodule=stateful_tcp_actor=5 dist/bin/idgs -c conf/cluster.conf  1>case31_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3

  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=stateful_tcp_actor=5 dist/bin/idgs -c conf/cluster.conf  1>case31_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3

  SSB_SIZE="0.1"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

  echo "generate ssb data"
  build/ssb-gen.sh


  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m integration_test/rdd_it/lineorder_mapper.conf -t 100 1>it_case31.log 2>&1
  
  sleep 5
  
  echo "run case... details refer to it_case31.log."
  $BUILD_DIR/target/itest/it_collect_action_test_big_data 1>>it_case31.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC).";
    exit $RC
  fi
  
  echo "kill servers..."
  safekill $SRV_PID1
  safekill $SRV_PID2
  check_core_dump dist/bin/idgs
}

case32() {
  echo "###############"	
  echo "Case 32: sum action test"
  echo "###############"	
  export GLOG_v=0
  
  echo "start server 1"
  export idgs_local_store=true
  export idgs_public_port=7700
  export idgs_inner_port=7701
  dist/bin/idgs -c conf/cluster.conf  1>case32_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3

  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case32_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3

  SSB_SIZE="0.001"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

  echo "generate ssb data"
  build/ssb-gen.sh

  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m integration_test/rdd_it/lineorder_mapper.conf -t 100 1>it_case32.log 2>&1

  sleep 5
  
  echo "run case... details refer to it_case32.log."
  $BUILD_DIR/target/itest/it_sum_action_test 1>>it_case32.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC).";
    exit $RC
  fi
  
  echo "kill servers..."
  safekill $SRV_PID1
  safekill $SRV_PID2
  check_core_dump dist/bin/idgs
}


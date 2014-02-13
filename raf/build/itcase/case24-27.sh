#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case24() {
  echo "########################"
  echo "Case 24: SSB Q1.1 test"
  echo " 1. start 2 servers"
  echo " 2. load ssb data, size 0.001G"
  echo " 3. run test"
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
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case24_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case24_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  SSB_Q1_LOOP=10
  export SSB_Q1_LOOP

  SSB_HOME="/tmp/ssb_it"
  export SSB_HOME
  SSB_SIZE="0.001"
  export SSB_SIZE

  echo "generate ssb data"
  build/ssb-gen.sh

  rm -f tpch-*.txt  
  echo "load ssb data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $SSB_HOME/ssb-dbgen-master -c framework/conf/cluster.conf -m samples/load/conf/ssb_file_mapper.conf -t 10 -o ssb-udptps.txt 1>it_case24.log 2>&1
  
  echo "run SSB Q1.1 test"
  dist/itest/ssb_Q1_1 1>ut_result.log 2>>it_case24.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case24.log.";
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
  
  # raw data result.
  time dist/itest/ssb_Q1_1_raw_data_result 1>ut_result.log 2>>it_case24.log
  echo ""
  echo "============ RESULT ============"
  cat ut_result.log
  echo "================================"
}

case25() {
  echo "########################"
  echo "Case 25: rdd name test"
  echo " 1. start 2 servers"
  echo " 2. load ssb data, size 0.001G"
  echo " 3. run test to count table lineorder"
  echo "########################"
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case25_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case25_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  SSB_HOME="/tmp/ssb_it"
  export SSB_HOME
  SSB_SIZE="0.001"
  export SSB_SIZE

  echo "generate ssb data"
  build/ssb-gen.sh

  rm -f tpch-*.txt  
  echo "load ssb data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $SSB_HOME/ssb-dbgen-master -c framework/conf/cluster.conf -m samples/load/conf/ssb_file_mapper.conf -t 10 -o ssb-udptps.txt 1>case25_load.log 2>&1
  
  echo "run rdd name test"
  dist/itest/it_rdd_name_test 1>ut_result.log 2>it_case25.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case25.log.";
    exit $RC
  fi

  echo "killing server 1"
  #safekill $SRV_PID1 
  #safekill $SRV_PID2

  echo ""
  echo "============ RESULT ============"
  cat ut_result.log
  echo "================================"
}

case26() {
  echo "########################"
  echo "Case 26: union transformer test"
  echo " 1. start 2 servers"
  echo " 2. load ssb data, size 0.001G"
  echo " 3. load tpch data, size 0.001G"
  echo " 3. run test to union lineitem of tpch and lineorder of ssb" 
  echo "        result field key(order_key, line_number), value(discount, price)"
  echo " 4. run count action and sum action"
  echo "########################"
  cd $WORKSPACE/idgs/

  
  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case26_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case26_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  SSB_HOME="/tmp/ssb_it"
  export SSB_HOME
  SSB_SIZE="0.001"
  export SSB_SIZE

  echo "generate ssb data"
  build/ssb-gen.sh
  
  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh


  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
    
  echo "load ssb data"
  dist/bin/load -s 1 -p $SSB_HOME/ssb-dbgen-master -c framework/conf/cluster.conf -m samples/load/conf/ssb_file_mapper.conf -t 10 -o ssb-udptps.txt 1>it_case26.log 2>&1
  
  echo "load tpch data"
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 -o tpch-udptps.txt 1>>it_case26.log 2>&1
  
  echo "run union transformer test"
  dist/itest/it_union_transformer_test 1>ut_result.log 2>>it_case26.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case26.log.";
    exit $RC
  fi

  echo "killing server 1"
  #safekill $SRV_PID1 
  #safekill $SRV_PID2

  echo ""
  echo "============ RESULT ============"
  cat ut_result.log
  echo "================================"
    
  # ensure kill all server
  #check_core_dump dist/bin/idgs
  #echo "########################"
}

case27() {
  echo "######################## Case 27: reducebykey transform ########################"
  cd $WORKSPACE/idgs/
  
  export GLOG_v=0
  
  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case27_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case27_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh

  rm -f tpch-*.txt
  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 -o tpch-udptps.txt 1>case27_load.log 2>&1
  
  sleep 5

  echo "########################run reducevalue transform test########################"
  dist/itest/it_reducebykey_transformer_test 1>case27_client.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC).";
    exit $RC
  fi

  rm -f tpch-*.txt

  echo "killing server 1"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  # ensure kill all server
  #check_core_dump dist/bin/idgs
  #echo "########################"
}


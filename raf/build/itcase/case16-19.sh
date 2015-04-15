#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case16() {
  echo "########################"
  echo "Case 16: Test client pool, assume pool size is 5, start 3 server, try to get all 5 client out from the pool, then recycle one client, and get 1 client again"
  echo "########################"
  cd $WORKSPACE/idgs/
  
  echo "start server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=8800
  dist/bin/idgs -c conf/cluster.conf  1>case16_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3
  
  echo "start server 2"
  export idgs_member_port=7701
  export idgs_member_innerPort=8801
  dist/bin/idgs -c conf/cluster.conf 1>case16_2.log 2>&1  &
  SRV_PID2=$!
  sleep 3
  
  echo "start server 3"
  export idgs_member_port=7702
  export idgs_member_innerPort=8802
  dist/bin/idgs -c conf/cluster.conf 1>case16_3.log 2>&1  &
  SRV_PID3=$!
  sleep 5
  
  echo "########################run client pool test########################"
  GLOG_v=10 run_test dist/itest/it_client_pool_test
  
  safekill $SRV_PID1 
  safekill $SRV_PID2
  safekill $SRV_PID3
  check_core_dump dist/bin/idgs
}

case17() {
  echo "########################"
  echo "Case 17: store delegate rdd test"
  echo " 1. start 2 servers"
  echo " 2. run test"
  echo "    test create store delegate"
  echo "    test count action of store delegate"
  echo "########################"
  cd $WORKSPACE/idgs/
  
  export GLOG_v=0
  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case17_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case17_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  export GLOG_v=0
  echo "insert 1000 data to store Orders"
  dist/bin/idgs-cli -f integration_test/rdd_it/insert_data 1>case17_load.log 2>&1

  export GLOG_v=10
  run_test dist/itest/it_store_delegate_rdd_test 
  
  echo "killing server 1"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  #echo "########################"
}

case18() {
  echo "########################"
  echo "Case 18: filter rdd test"
  echo " 1. start 2 servers"
  echo " 2. run test"
  echo "    test create filter"
  echo "    test count action of filter"
  echo "    test count action of filter with last filter result"
  echo "########################"
  cd $WORKSPACE/idgs/
  export GLOG_v=1
  
  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7702
  export idgs_member_service_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case18_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8802
  dist/bin/idgs -c conf/cluster.conf  1>case18_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  echo "insert 1000 data to store Orders"
  dist/bin/idgs-cli -f integration_test/rdd_it/insert_data 1>case18_3.log 2>&1

  echo "filter RDD"
  run_test dist/itest/it_filter_rdd_test 
  
  echo "killing server 1, 2"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  check_core_dump dist/bin/idgs
}

case19() {
  echo "########################"
  echo "Case 19: tpch Q6 test"
  echo " 1. start 2 servers"
  echo " 2. load tpch data, size 0.01"
  echo " 3. run test"
  echo "    select sum(l_extendedprice * l_discount)"
  echo "      from lineitem"
  echo "     where l_shipdate <= '1994-01-01'"
  echo "       and l_shipdate > '1995-01-01'"
  echo "       and l_discount between 0.05 and 0.07"
  echo "       and l_quantity < 24"
  echo "    execute sql result 10000 times"
  echo "########################"
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case19_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  GLOG_vmodule=*rdd*=3 dist/bin/idgs -c conf/cluster.conf  1>case19_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_Q6_LOOP=100
  export TPCH_Q6_LOOP

  TPCH_SIZE="0.001"
  export TPCH_SIZE
  TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
  export TPCH_HOME

  echo "generate tpch data"
  build/tpch-gen.sh


  rm -f tpch-*.txt
  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 10 -o tpch-udptps.txt 1>case19_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case19.log.";
    exit $RC
  fi
  
  #exit
  sleep 1

  echo "run tpch Q6 test"
  dist/itest/tpch_Q6_1 2>ut_result.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  rm -f tpch-*.txt

  echo ""
  echo "============ RESULT ============"
  cat ut_result.log
  echo "================================"

  echo "killing server 1"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  # ensure kill all server
  check_core_dump dist/bin/idgs
  #echo "########################"

  # raw data result.
  dist/itest/tpch_Q6_raw_data_result 2>>it_case19.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case19.log.";
    exit $RC
  fi
}


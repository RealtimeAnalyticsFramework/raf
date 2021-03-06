#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case33() {
  echo "########################"
  echo "Case 33: Top N Action Test"
  echo " 1. start 2 servers"
  echo " 2. load tpch data, size 0.001G"
  echo " 3. run test by store LineItem"
  echo "    top 10 order by l_extendedprice"
  echo "    top 10 order by l_extendedprice desc"
  echo "    top 10 order by l_extendedprice * l_discount"
  echo "    top 5 start with index 5 order by l_extendedprice "
  echo "########################"
  cd $WORKSPACE/idgs/
  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case33_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case33_2.log 2>&1 &
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
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 -o tpch-udptps.txt 1>it_case33.log 2>&1

  sleep 1

  echo "run top N action test"
  $BUILD_DIR/target/itest/it_top_n_action_test 1>ut_result.log 2>>it_case33.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case33.log.";
    exit $RC
  fi

  rm -f tpch-*.txt

  echo "killing server 1"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  # ensure kill all server
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case34() {
  echo "########################"
  echo "Case 34: If expression Test"
  echo " 1. start 2 servers"
  echo " 2. load tpch data, size 0.001G"
  echo " 3. run test by store LineItem"
  echo "    select l_orderkey,"
  echo "           l_linenumber,"
  echo "           l_quantity, "
  echo "           case when l_quantity > 24 then 'quantity > 24'"
  echo "                when 10 < l_quantity <= 24 then '10 < quantity <= 24'"
  echo "                else 'quantity <= 10'"
  echo "            end quantity_desc"
  echo "      from lineitem"
  echo "########################"
  cd $WORKSPACE/idgs/
  export GLOG_v=0
  

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_v=1 dist/bin/idgs -c conf/cluster.conf  1>case34_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_v=1 dist/bin/idgs -c conf/cluster.conf  1>case34_2.log 2>&1 &
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
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 -o tpch-udptps.txt 1>it_case34.log 2>&1

  sleep 1

  echo "run If expression test"
  $BUILD_DIR/target/itest/it_if_expression_test 1>ut_result.log 2>>it_case34.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC), refer to it_case19.log.";
    exit $RC
  fi

  rm -f tpch-*.txt

  echo "killing server 1"
  safekill $SRV_PID1 
  safekill $SRV_PID2
  
  # ensure kill all server
  check_core_dump dist/bin/idgs
  #echo "########################"
}

case36() {
  echo "########################"
  echo "Case 36: Java Client Integration Test"
  echo " 1. start 2 server"
  echo " 2. run java client, insert a customer into store"
  echo "########################"
  if ! which mvn; then echo "skip"; return; fi
  
  echo "start server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_v=1 dist/bin/idgs -c conf/cluster.conf  1>case36_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3   
  
  echo "start server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  GLOG_v=1 dist/bin/idgs -c conf/cluster.conf  1>case36_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3 
  
  echo "run java client, insert a customer into store" 
  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-java
  mvn test -Dtest=**/*IT.java

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
}

case37() {
  echo "#####################################################################"
  echo "Case 37: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test select * "
  echo "           from orders "
  echo "          where o_orderstatus = 'F'"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case37_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case37_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_SIZE="0.001"
  export TPCH_SIZE
  TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
  export TPCH_HOME

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case37_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-sql
  echo "run select test"
  mvn test -Dtest=integration/SelectTableIT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
}

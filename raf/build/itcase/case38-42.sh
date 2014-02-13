#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case38() {
  echo "#####################################################################"
  echo "Case 38: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test group by by select l_suppkey, count(1) cnt "
  echo "                       from lineitem "
  echo "                   group by l_suppkey"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case38_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case38_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 1>case38_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/sql_engine
  echo "run group by test"
  mvn test -Dtest=integration/GroupByIT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
  check_core_dump dist/bin/idgs 
}

case39() {
  echo "#####################################################################"
  echo "Case 39: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test union by select * "
  echo "                    from (select s_comment comment "
  echo "                            from supplier "
  echo "                       union all "
  echo "                          select o_comment comment "
  echo "                            from orders"
  echo "                         ) t"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case39_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case39_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 1>case39_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/sql_engine
  echo "run union test"
  mvn test -Dtest=integration/UnionIT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
  check_core_dump dist/bin/idgs 
}

case40() {
  echo "#####################################################################"
  echo "Case 40: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test join by select o.o_orderkey, l.l_orderkey, o.o_orderstatus, l.l_discount "
  echo "                   from orders o left outer join lineitem l "
  echo "                     on o.o_orderkey = l.l_orderkey "
  echo "                  where o.o_orderstatus = 'F' "
  echo "                    and l.l_discount between 0.05 and 0.07"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case40_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case40_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 1>case40_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/sql_engine
  echo "run hash join test"
  mvn test -Dtest=integration/HashJoinIT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
  check_core_dump dist/bin/idgs 
}

case41() {
  echo "#####################################################################"
  echo "Case 41: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test order by select * from customer t order by c_name"
  echo "              and select * from customer t order by c_name desc"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case41_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case41_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  TPCH_HOME="/tmp/tpch_it"
  export TPCH_HOME
  TPCH_SIZE="0.001"
  export TPCH_SIZE

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $TPCH_HOME/dbgen -c framework/conf/cluster.conf -m samples/load/conf/tpch_file_mapper.conf -t 10 1>case41_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/sql_engine
  echo "run group by test"
  mvn test -Dtest=integration/OrderByIT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  cd $CURR_DIR
  check_core_dump dist/bin/idgs 
}

case42() {
  echo "#####################################################################"
  echo "Case 42: Sql engine Integration Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test sql "
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
}

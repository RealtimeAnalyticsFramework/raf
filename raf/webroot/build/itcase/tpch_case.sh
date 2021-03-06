#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case_tpchQ6() {
  echo "#####################################################################"
  echo "Case Q6: Sql engine Integration Test TPCH Q6"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run sql engine"
  echo "    test tpch Q6 select sum(l_extendedprice * l_discount) price "
  echo "                   from iineitem "
  echo "                  where l_shipdate >= '1994-01-01' "
  echo "                    and l_shipdate < '1995-01-01' "
  echo "                    and l_discount between 0.05 and 0.07 "
  echo "                    and l_quantity < 24'"
  echo "#####################################################################"
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>caseQ6_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>caseQ6_2.log 2>&1 &
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>caseQ6_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-sql
  echo "run tpch Q6 test"
  mvn test -Dtest=integration/tpch/TpchQ6IT.java
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

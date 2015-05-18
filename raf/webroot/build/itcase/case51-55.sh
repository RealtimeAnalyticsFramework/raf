#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case51() {
  echo "#####################################################################"
  echo "Case 51: test migration and backup when loading data"
  echo " 1. start server 1"
  echo " 2. start server 2"
  echo " 3. load test data"
  echo " 4. sleep to wait for insert some data"
  echo " 5. start server 3"
  echo " 6. sleep to wait migration and backup done"
  echo " 7. check migration"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*migration_*=1 dist/bin/idgs -c conf/sync_cluster.conf  1>case51_1.log 2>&1 &
  SRV_PID1=$!
  sleep 1
  
  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  GLOG_vmodule=*migration_*=1 dist/bin/idgs -c conf/sync_cluster.conf  1>case51_2.log 2>&1 &
  SRV_PID2=$!
  sleep 1
  
  TPCH_SIZE="0.1"
  export TPCH_SIZE
  TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
  export TPCH_HOME

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data by 2 servers"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case51_load.log 2>&1 &
  
  echo "wait for insert some data"
  sleep 15
  
  echo "starting server 3"
  export idgs_public_port=8803
  export idgs_inner_port=8804
  export idgs_local_store=true
  GLOG_vmodule=*migration_*=1 dist/bin/idgs -c conf/sync_cluster.conf  1>case51_3.log 2>&1 &
  SRV_PID3=$!
  
  echo "wait migration and backup done"
  sleep 150
  
  echo "check migration of 3 servers"
  $BUILD_DIR/target/itest/migration_verify 2>it_case51.log || exit $?
  
  safekill $SRV_PID1
  safekill $SRV_PID2
  safekill $SRV_PID3

  check_core_dump dist/bin/idgs 
}

#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

case43() {
  echo "#####################################################################"
  echo "Case 43: RDD destroy test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run tpchQ6"
  echo " 4. call destroy and verify"
  echo "#####################################################################"
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case43_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case43_2.log 2>&1 &
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case43_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  echo "run RDD destroy test"
  $BUILD_DIR/target/itest/it_rdd_destroy_test 1>ut_result.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  check_core_dump dist/bin/idgs 
}

case44() {
  echo "#####################################################################"
  echo "Case 44: JDBC test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run jdbc test"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*rdd*=2 dist/bin/idgs -c conf/cluster.conf  1>case44_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=*rdd*=2 dist/bin/idgs -c conf/cluster.conf  1>case44_2.log 2>&1 &
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case44_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-jdbc
  echo "run jdbc test"
  ulimit -n 65536
  mvn test -Dtest=idgs/jdbc/*IT.java
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

case45() {
  echo "#####################################################################"
  echo "Case 45: RDD DAG test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run DAG test"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case45_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case45_2.log 2>&1 &
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case45_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  echo "run DAG test"
  $BUILD_DIR/target/itest/it_rdd_dag_test
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  check_core_dump dist/bin/idgs 
}

case46() {
  echo "#####################################################################"
  echo "Case 46: RDD Store Listener Test"
  echo " 1. start 2 server"
  echo " 2. load tpch data size 0.01"
  echo " 3. run RDD store listener test"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case46_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  dist/bin/idgs -c conf/cluster.conf  1>case46_2.log 2>&1 &
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case46_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 1

  echo "run RDD store listener test"
  $BUILD_DIR/target/itest/it_rdd_store_listener_test
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  check_core_dump dist/bin/idgs 
}

case47() {
  echo "#####################################################################"
  echo "Case 47: Store schema Test"
  echo " 1. start 2 server"
  echo " 2. run store schema test"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case47_1.log 2>&1 &
  SRV_PID1=$!
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case47_2.log 2>&1 &
  SRV_PID2=$!
  sleep 2

  echo "run store schema test"
  $BUILD_DIR/target/itest/it_store_schema_test 1>ut_result.log 2>it_case47.log
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  check_core_dump dist/bin/idgs
}

case48() {
  echo "#####################################################################"
  echo "Case 48: Migration Test"
  echo " 1. start server 1"
  echo " 2. load tpch data"
  echo " 3. start server 2"
  echo " 4. start server 3"
  echo " 5. kill server 2"
  echo " 6. restart server 2"
  echo " 7. kill server 3"
  echo " 8. kill server 2"
  echo " 9. kill server 1"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "1. starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=store_migration_*=2 dist/bin/idgs -c conf/cluster.conf  1>case48_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3
  
  TPCH_SIZE="0.001"
  export TPCH_SIZE
  TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
  export TPCH_HOME

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "2. load tpch data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case48_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_1.log || exit $?

  echo "3. starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  GLOG_vmodule=store_migration_*=2 dist/bin/idgs -c conf/cluster.conf  1>case48_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_2.log  || exit $?
 
  echo "4. starting server 3" 
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=true
  GLOG_vmodule=store_migration_*=2 dist/bin/idgs -c conf/cluster.conf  1>case48_3.log 2>&1 &
  SRV_PID3=$!
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_3.log || exit $?

  echo "5. kill servers 2."
  safekill $SRV_PID2
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_4.log || exit $?
  
  echo "6. restarting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  GLOG_vmodule=store_migration_*=2 dist/bin/idgs -c conf/cluster.conf  1>case48_4.log 2>&1 &
  SRV_PID2=$!
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_5.log || exit $?
  
  echo "7. kill server 3"
  safekill $SRV_PID3
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_6.log || exit $?
  
  echo "8. kill server 2"
  safekill $SRV_PID2
  sleep 3
  
  $BUILD_DIR/target/itest/migration_verify 2>case48_migration_7.log || exit $?

  echo "9. kill server 1"
  safekill $SRV_PID1

  check_core_dump dist/bin/idgs 
}

case49() {
  echo "#####################################################################"
  echo "Case 49: Sync Test"
  echo " 1. start server 1"
  echo " 2. load tpch data"
  echo " 3. start server 2"
  echo " 4. start server 3"
  echo " 5. kill server 2"
  echo " 6. restart server 2"
  echo " 7. kill server 3"
  echo " 8. kill server 2"
  echo " 9. kill server 1"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case49_1.log 2>&1 &
  SRV_PID1=$!
  sleep 3
  
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
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case49_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_1.log

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case49_2.log 2>&1 &
  SRV_PID2=$!
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_2.log
 
  echo "starting server 3" 
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case49_3.log 2>&1 &
  SRV_PID3=$!
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_3.log

  echo "kill servers 2."
  safekill $SRV_PID2
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_4.log
  
  echo "restarting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  dist/bin/idgs -c conf/cluster.conf  1>case49_4.log 2>&1 &
  SRV_PID2=$!
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_5.log
  
  echo "kill server 3"
  safekill $SRV_PID3
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_6.log
  
  echo "kill server 2"
  safekill $SRV_PID2
  sleep 3
  
  $BUILD_DIR/target/itest/sync_verify 2>case49_sync_7.log

  safekill $SRV_PID1

  check_core_dump dist/bin/idgs 
}

case50() {
  echo "#####################################################################"
  echo "Case 50: backup listener test"
  echo "  1. start server 1"
  echo "  2. start server 2"
  echo "  3. load test data"
  echo "  4. check backup result"
  echo "  5. load tpch data by 2 servers"
  echo "  6. check backup result"
  echo "  7. start server 3"
  echo "  8. check migration"
  echo "  9. reload tpch data by 3 servers"
  echo " 10. check backup result"
  echo "#####################################################################"
  if ! which mvn; then echo "skip"; return; fi
  cd $WORKSPACE/idgs/

  export GLOG_v=0

  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  dist/bin/idgs -c conf/sync_cluster.conf  1>case50_1.log 2>&1 &
  SRV_PID1=$!
  sleep 1
  
  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  export idgs_local_store=true
  dist/bin/idgs -c conf/sync_cluster.conf  1>case50_2.log 2>&1 &
  SRV_PID2=$!
  sleep 1
  
  echo "load test data include insert, update and delete"
  dist/bin/idgs-cli -f integration_test/store_it/test_backup_data 2>it_test_data_load.log || exit $?
  sleep 3
  
  echo "check backup result"
  $BUILD_DIR/target/itest/migration_verify 2>it_case50_0.log || exit $?
  
  TPCH_SIZE="0.005"
  export TPCH_SIZE
  TPCH_HOME="/tmp/tpch_$TPCH_SIZE"
  export TPCH_HOME

  echo "generate tpch data"
  build/tpch-gen.sh

  echo "load tpch data by 2 servers"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case50_load.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  sleep 3
  
  echo "check backup result"
  $BUILD_DIR/target/itest/migration_verify 2>it_case50_1.log || exit $?
  
  echo "starting server 3"
  export idgs_public_port=8803
  export idgs_inner_port=8804
  export idgs_local_store=true
  dist/bin/idgs -c conf/sync_cluster.conf  1>case50_3.log 2>&1 &
  SRV_PID3=$!
  sleep 3
  
  echo "check migration of 3 servers"
  $BUILD_DIR/target/itest/migration_verify 2>it_case50_2.log || exit $?
  
  echo "truncate store lineitem"
  $BUILD_DIR/target/itest/it_truncate_store_test
  
  echo "check backup"
  $BUILD_DIR/target/itest/migration_verify 2>it_case50_3.log || exit $?
  
  echo "reload tpch data by 3 servers"
  dist/bin/idgs-load -s 1 -p $TPCH_HOME/dbgen -c conf/cluster.conf -m conf/tpch_file_mapper.conf -t 100 1>case50_load.log 2>&1
  sleep 5

  echo "check backup result"
  $BUILD_DIR/target/itest/migration_verify 2>it_case50_4.log || exit $?
  
  safekill $SRV_PID1
  safekill $SRV_PID2
  safekill $SRV_PID3

  check_core_dump dist/bin/idgs 
}

#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#

LOAD_THREAD=150

ssb_case() {
  IT_CASE_NAME=$1
  IT_TEST_NAME=$2
  echo "#####################################################################"
  echo "Case SsbQ${IT_CASE_NAME}: Sql engine Integration Test SSB Q${IT_CASE_NAME}"
  echo " 1. start 2 server"
  echo " 2. load ssb data"
  echo " 3. run sql engine test ssb Q${IT_CASE_NAME}"
  echo "#####################################################################"
  export GLOG_v=0
  export DEFAULT_PERSIST_TYPE=NONE
  export CLIENT_TIMEOUT=900000
  
  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*rdd*=1 dist/bin/idgs -c conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_1.log 2>&1 &
  SRV_PID1=$!
  export SRV_PID1
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=*rdd*=1 dist/bin/idgs -c conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_2.log 2>&1 &
  SRV_PID2=$!
  export SRV_PID2
  sleep 2

  SSB_SIZE="1"
  export SSB_SIZE
  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME
  
  echo "generate ssb data"
  build/ssb-gen-it.sh

  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m conf/ssb_file_mapper.conf -t $LOAD_THREAD 1>load_ssb_caseQ${IT_CASE_NAME}.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 2
  
  rm /tmp/root/idgs.log
  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-sql
  echo "run ssb Q${IT_CASE_NAME} test"
  mvn test -Dtest=integration/ssb/SsbQ${IT_TEST_NAME}IT.java
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi

  echo "kill servers."
  safekill $SRV_PID1
  safekill $SRV_PID2

  unset SRV_PID1
  unset SRV_PID2
  unset CHECK_RAW_RESULT
  
  cd $CURR_DIR
  check_core_dump dist/bin/idgs 
  echo 3 > /proc/sys/vm/drop_caches
}

ssb_case1() {
  IT_CASE_NAME=$1
  echo "#####################################################################"
  echo "Case Ssb Q${IT_CASE_NAME}: Sql engine Integration Test SSB"
  echo " 1. start 2 server"
  echo " 2. load ssb data"
  echo " 3. run sql engine test ssb"
  echo "#####################################################################"
  export GLOG_v=0
  export DEFAULT_PERSIST_TYPE=NONE
  export CLIENT_TIMEOUT=900000
  
  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*rdd*=1 dist/bin/idgs -c conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_1.log 2>&1 &
  SRV_PID1=$!
  export SRV_PID1
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=*rdd*=1 dist/bin/idgs -c conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_2.log 2>&1 &
  SRV_PID2=$!
  export SRV_PID2
  sleep 2

  SSB_SIZE="1"
  export SSB_SIZE

  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME

    
  echo "generate ssb data"
  build/ssb-gen.sh

  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m conf/ssb_file_mapper.conf -t $LOAD_THREAD 1>load_case_ssbQ${IT_CASE_NAME}.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 2
  
  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-sql
  echo "run ssb test"
  mvn test -Dtest=integration/ssb/SsbQ${IT_CASE_NAME}_*IT.java
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
  echo 3 > /proc/sys/vm/drop_caches
}

ssbcase() {
  ssb_case 1.1 1_1
  ssb_case 1.2 1_2
  ssb_case 1.3 1_3
  ssb_case 2.1 2_1
  ssb_case 2.2 2_2
  ssb_case 2.3 2_3
  ssb_case 3.1 3_1
  ssb_case 3.2 3_2
  ssb_case 3.3 3_3
  ssb_case 3.4 3_4
  ssb_case 4.1 4_1
  ssb_case 4.2 4_2
  ssb_case 4.3 4_3
}

ssbcase1() {
  ssb_case1 1
  ssb_case1 2
  ssb_case1 3
  ssb_case1 4
}

ssbcase2() {
  echo "#####################################################################"
  echo "Case Ssb : Sql engine Integration Test SSB"
  echo " 1. start 2 server"
  echo " 2. load ssb data"
  echo " 3. run sql engine test ssb"
  echo "#####################################################################"
  export GLOG_v=0
  export DEFAULT_PERSIST_TYPE=NONE
  export CLIENT_TIMEOUT=900000
  
  echo "starting server 1"
  export idgs_public_port=7700
  export idgs_inner_port=7701
  export idgs_local_store=true
  GLOG_vmodule=*rdd*=2 dist/bin/idgs -c conf/cluster.conf  1>case_ssb_1.log 2>&1 &
  SRV_PID1=$!
  export SRV_PID1
  sleep 2

  echo "starting server 2"
  export idgs_public_port=8800
  export idgs_inner_port=8801
  GLOG_vmodule=*rdd*=2 dist/bin/idgs -c conf/cluster.conf  1>case_ssb_2.log 2>&1 &
  SRV_PID2=$!
  export SRV_PID2
  sleep 2

  SSB_SIZE="1"
  export SSB_SIZE

  SSB_HOME="/tmp/ssb_$SSB_SIZE"
  export SSB_HOME
    
  echo "generate ssb data"
  build/ssb-gen.sh

  echo "load ssb data"
  export idgs_public_port=9900
  export idgs_inner_port=9901
  export idgs_local_store=false
  dist/bin/idgs-load -s 1 -p $SSB_HOME/ssb-dbgen-master -c conf/cluster.conf -m conf/ssb_file_mapper.conf -t $LOAD_THREAD 1>load_case_ssb.log 2>&1
  #$BUILD_DIR/target/itest/it_load_data_test
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 2
  
  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/idgs-sql
  echo "run ssb test"
  mvn test -Dtest=integration/ssb/SsbQ*IT.java
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
  echo 3 > /proc/sys/vm/drop_caches
}

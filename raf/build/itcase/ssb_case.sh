#!/bin/sh
#
# Script to run unit test, invoked by jenkins
#
run_ssb_case() {
  IT_CASE_NAME=$1
  IT_TEST_NAME=$2
  echo "#####################################################################"
  echo "Case SsbQ${IT_CASE_NAME}: Sql engine Integration Test SSB Q${IT_CASE_NAME}"
  echo " 1. start 2 server"
  echo " 2. load ssb data"
  echo " 3. run sql engine test ssb Q${IT_CASE_NAME}"
  echo "#####################################################################"
  export GLOG_v=0
  #export CHECK_RAW_RESULT=true
  
  rm /tmp/root/idgs.log
  
  echo "starting server 1"
  export idgs_member_port=7700
  export idgs_member_innerPort=7701
  export idgs_member_service_local_store=true
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_1.log 2>&1 &
  SRV_PID1=$!
  export SRV_PID1
  sleep 2

  echo "starting server 2"
  export idgs_member_port=8800
  export idgs_member_innerPort=8801
  dist/bin/idgs-aio -c framework/conf/cluster.conf  1>case_ssbQ${IT_CASE_NAME}_2.log 2>&1 &
  SRV_PID2=$!
  export SRV_PID2
  sleep 2

  SSB_HOME="/tmp/ssb_it"
  export SSB_HOME
  
  echo "generate ssb data"
  build/ssb-gen-it.sh

  echo "load ssb data"
  export idgs_member_port=9900
  export idgs_member_innerPort=9901
  export idgs_member_service_local_store=false
  dist/bin/load -s 1 -p $SSB_HOME/ssb-dbgen-master -c framework/conf/cluster.conf -m samples/load/conf/ssb_file_mapper.conf -t 10 1>it_ssb_caseQ${IT_CASE_NAME}.log 2>&1
  RC=$?
  if [ $RC -ne 0 ] ; then
    echo "Abnormal exit (RC=$RC)";
    exit $RC
  fi
  
  sleep 2
  
  rm /tmp/root/idgs.log
  CURR_DIR=`pwd`
  cd $WORKSPACE/idgs/front_end/sql_engine
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

ssbcase() {
  rm $SSB_HOME -rf
  ssbQ1.1
  ssbQ1.2
  ssbQ1.3
  ssbQ2.1
  ssbQ2.2
  ssbQ2.3
  ssbQ3.1
  ssbQ3.2
  ssbQ3.3
  ssbQ3.4
  ssbQ4.1
  ssbQ4.2
  ssbQ4.3
}

ssbQ1.1() {
  run_ssb_case 1.1 1_1
}

ssbQ1.2() {
  run_ssb_case 1.2 1_2
}

ssbQ1.3() {
  run_ssb_case 1.3 1_3
}

ssbQ2.1() {
  run_ssb_case 2.1 2_1
}

ssbQ2.2() {
  run_ssb_case 2.2 2_2
}

ssbQ2.3() {
  run_ssb_case 2.3 2_3
}

ssbQ3.1() {
  run_ssb_case 3.1 3_1
}

ssbQ3.2() {
  run_ssb_case 3.2 3_2
}

ssbQ3.3() {
  run_ssb_case 3.3 3_3
}

ssbQ3.4() {
  run_ssb_case 3.4 3_4
}

ssbQ4.1() {
  run_ssb_case 4.1 4_1
}

ssbQ4.2() {
  run_ssb_case 4.2 4_2
}

ssbQ4.3() {
  run_ssb_case 4.3 4_3
}
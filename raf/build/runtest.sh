#!/bin/bash
#
# Script to run unit test, invoked by jenkins
#

# exit when failure
set +e

if [ "$WORKSPACE" = "" ]; then
  WORKSPACE=`pwd`
  WORKSPACE="$WORKSPACE/.."
  export WORKSPACE
fi
if [ "$BUILD_DIR" = "" ]; then
  BUILD_DIR=$WORKSPACE/idgs
  export BUILD_DIR
fi
echo "====== WORKSPACE: $WORKSPACE ======="

# killall idgs 2>/dev/null


echo "############################## Setup environment ###############################################"
ulimit -c unlimited
# remove all core dump.
find . -name "core.*" -exec rm -f {} \; 2>/dev/null
rm -f hive_job_log*.txt 2>/dev/null

# turn on verbose log
GLOG_v=5
export GLOG_v 
GLOG_vmodule="main=0"
export GLOG_vmodule
export GTEST_COLOR="yes"

#
# run a unit test
# $1: test case 
#
run_test() {
  # $1 || exit $?
  echo "######### Unit Test:  $1  #############"
  $1 --gtest_color=yes 2>ut.log
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    exit $RC_CODE
  fi
}

ensure_corosync(){
  echo "ensure corosync"
  # service corosync stop
  service corosync start
}

ensure_corosync

cd $WORKSPACE/idgs

rm ./dist/utest/cluster_cfg_parser_env_test  

# register EXIT trap
. build/archive_test_log.sh

# run all unit cases.
UT_CASES=`dir ./dist/utest`
for UT in $UT_CASES; do
  rm *.log 2>/dev/null
  IT_CASE_NAME=$UT
  export IT_CASE_NAME

  run_test ./dist/utest/$UT
done
rm *.log 2>/dev/null



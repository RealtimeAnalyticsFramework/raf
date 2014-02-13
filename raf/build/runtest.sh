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
  export $BUILD_DIR
fi
echo "====== WORKSPACE: $WORKSPACE ======="

/bin/kill -9 idgs 2>/dev/null
/bin/kill -9 idgs-aio 2>/dev/null
ps -ef | grep lt-it | grep -v grep | awk --source '{print $2}' | xargs /bin/kill -9
ps -ef | grep lt-ut | grep -v grep | awk --source '{print $2}' | xargs /bin/kill -9



echo "############################## Setup environment ###############################################"
ulimit -c unlimited
# remove all core dump.
find . -name "core.*" -exec rm -f {} \; 2>/dev/null

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
    echo "####################### dump stderr ###############################"
    cat ut.log
    echo "###################################################################"
   
    CORE_FILE=`ls core.* 2>/dev/null`
    if [ "$CORE_FILE" != "" ] ; then
      # ldd $1
      echo "####################### Core Dump Stack ###############################"
      gdb -ex where -ex quit $1 $CORE_FILE
    fi

    exit $RC_CODE
  fi
}

ensure_corosync(){
  echo "restart corosync"
  service corosync stop
  service corosync start
}

ensure_corosync

rm ./dist/utest/cluster_cfg_parser_env_test  

UT_CASES=`dir ./dist/utest`

for UT in $UT_CASES; do
  run_test ./dist/utest/$UT
done


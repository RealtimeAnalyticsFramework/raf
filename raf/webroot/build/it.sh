#!/bin/bash
#
# Script to run unit test, invoked by jenkins
#

if [ "$WORKSPACE" = "" ]; then
  WORKSPACE=`pwd`
  WORKSPACE="$WORKSPACE/.."
  export WORKSPACE
else
  cd $WORKSPACE/idgs
fi

if [ "$BUILD_DIR" == "" ]; then
  export BUILD_DIR=$WORKSPACE/idgs
fi
echo "WORKSPACE: $WORKSPACE"
if [ "$IDGS_HOME" = "" ]; then
  IDGS_HOME=$WORKSPACE/idgs/dist
  export IDGS_HOME
fi

killall -9 idgs 2>/dev/null 1>&2

echo "############################## Begin Setup Globle environment ##################"

ulimit -c unlimited

# turn on verbose log
export GLOG_v=5
export GLOG_vmodule="main=0"
export GTEST_COLOR="yes"

export IDGS_LOG_DIR=$WORKSPACE/idgs
export MAVEN_OPTS="-Dhive.log.dir=$WORKSPACE/idgs -Dhive.querylog.location=$WORKSPACE/idgs -Dhive.exec.scratchdir=$WORKSPACE/idgs"

VALGRIND=""
if [ -f `which valgrind` ] ; then
  #VALGRIND="valgrind --leak-check=full --suppressions=build/valgrind/dlopen.txt --suppressions=build/valgrind/idgs.txt"
  VALGRIND="valgrind --suppressions=build/valgrind/dlopen.txt --suppressions=build/valgrind/idgs.txt"
  echo "valgrind"
fi


echo "############################## End Setup Globle environment ##################"

# register EXIT trap
. build/archive_test_log.sh

#
# safekill child proceses
# arguments:
#  $1: pid
#
KILL_TIMEOUT=60
export KILL_TIMEOUT
safekill() {
  #WPID=`/bin/kill -p $@`
  WPID="$@"
  export WPID

#  echo "safekill $WPID, timeout: $KILL_TIMEOUT"

  # start timer
  (sleep $KILL_TIMEOUT; echo "[DEAD LOCK]; force to kill $WPID"; /bin/kill -9 $WPID; exit 1) &
  # record timer pid
  TPID=$!
  export TPID

  /bin/kill -s SIGINT $WPID
  wait $WPID
 
  # kill timer process 
  kill $TPID 2>/dev/null
  wait $TPID 1>/dev/null 2>&1
  TRC=$?
  #echo "timer process return $TRC"
  if [ $TRC -eq 1 ] ; then
    echo "force shutdown";
    # dump logs and exit
    return 1
  fi

  return 0 
}

imm_safekill() {
  WPID="$@"
  export WPID

  # start timer
  (sleep $KILL_TIMEOUT; echo "[DEAD LOCK]; force to kill $WPID"; /bin/kill -9 $WPID; exit 1) &
  # record timer pid
  TPID=$!
  export TPID

  KRC=1
  while [[ $KRC -eq 1 ]]
  do
    KRC=`ps --no-heading $WPID | wc -l` 
    /bin/kill -s SIGINT $WPID
  done
  wait $WPID
 
  # kill timer process 
  kill $TPID 2>/dev/null
  wait $TPID 1>/dev/null 2>&1
  TRC=$?
  #echo "timer process return $TRC"
  if [ $TRC -eq 1 ] ; then
    echo "force shutdown";
    # dump logs and exit
    return 1
  fi

  return 0 
}


#
# run a unit test
# $1: test case 
#
run_test() {
  # $1 || exit $?
  echo "run test:  $1"
  $1 1>ut.log 2>&1
  RC_CODE=$?
  if test $RC_CODE -ne 0 ; then
    echo "Test case exit with code: $RC_CODE"
    if test $RC_CODE -ne 1 ; then
      CORE_FILE=`ls core* 2>/dev/null`
      if [ "$CORE_FILE" != "" ] ; then
        # ldd $1
        #cp core* $WORKSPACE/../builds/$BUILD_NUMBER
  
        echo "####################### Core Dump Stack ###############################"
        gdb -ex "thread apply all bt" -ex quit $1 $CORE_FILE
        echo "####################### log content ###############################"
      fi
    else
      check_core_dump dist/bin/idgs
      exit $RC_CODE
    fi

    exit $RC_CODE
  fi
}

#
# $1: executable file
#
check_core_dump() {
  CORE_FILE=`ls core* 2>/dev/null`
  if [ "$CORE_FILE" != "" ] ; then
    #echo "####################### Core Dump Stack #############"
    #gdb -ex where -ex quit $1 $CORE_FILE
    #echo "########################"
    
    #cp core* $WORKSPACE/../builds/$BUILD_NUMBER

    exit 1
  fi
}

echo `pwd`
for CASE in `ls build/itcase/*.sh`; do
  echo "source $CASE"
  #source $CASE
  . $CASE
done

ensure_corosync(){
  echo "ensure corosync"
  # service corosync stop >/dev/null
  service corosync start >/dev/null
}

#
# run an integration test case
# $1: case
#
it_case() {
  IT_CASE_NAME=$1
  jobs -p | xargs kill -9  >/dev/null 2>&1

  # killall -9 idgs 2>/dev/null 1>&2

  rm -f *.log 2>/dev/null
  rm -f core* 2>/dev/null
  rm -f hive_job_log*.txt

  ensure_corosync
  export GLOG_v=5
  $1
  #echo "$1"
  check_core_dump
}

#
# if case is specified, only the case is invoked. e.g.
#  build/it.sh case2
#
if [ "$#" -eq 1 ]; then
  echo "run specified case: $1"
  it_case $1
  exit 0
fi

#
# run all test cases
#
ITCASES=`cat <<END
#===================================================================================== 
# cluster
case0
case1
case2
case14 
case15

# rpc and actor
case4
case8
case13
case16
case21
case22

# store and client
case6
case7
case9
case10
case11
case12
case20
case47

# rdd
case17
case18
case23
case25
case26
case43

# tpc
case19
case24

# reducebykey 
case27
case28

# lookup action 
case29

# collect action 
case30

# count action 
case31

# sum action 
case32

case33
case34

# DAG
case45

# java client
case36

# sql engine
case37
case38
case39
case40
case41
case42

# jdbc
case44

# rdd store listener
case46

# migration
#case48
#case49

# buckup
#case50

#===================================================================================== 
END`

# echo $ITCASES
for ITC in $ITCASES ; do
  # echo $ITC
  if [[ $ITC == case* ]] ; then
    it_case $ITC
  fi
done

# clean up
#rm -f *.log 2>/dev/null
jobs -p | xargs kill -9  >/dev/null 2>&1
echo "IT success!"
exit 0

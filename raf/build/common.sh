#!/bin/bash

# WORKSPACE
if test "$WORKSPACE" = "" ; then
  WORKSPACE=`pwd`
  WORKSPACE=$WORKSPACE/..
  export WORKSPACE
fi

# BUILD_DIR
if test "$BUILD_DIR" = "" ; then
  BUILD_DIR=$WORKSPACE/idgs
  export BUILD_DIR
fi

ulimit -c unlimited

run_bg () {
  declare -i GOT_TRAP=0
  trap "GOT_TRAP=1" SIGINT
  trap "GOT_TRAP=1" SIGTERM
  trap "GOT_TRAP=1" SIGKILL
  echo "exec: $@"
  $@ &
  EXEC_PID=$!
  echo "APP PID="$EXEC_PID
  wait $EXEC_PID
  EC=$?
  if test $GOT_TRAP -eq 1 ; then
    kill $EXEC_PID
    wait $EXEC_PID
    EC=$?
  fi
  if test $EC -eq 0 ; then
    # success
    echo "Success!"
    return 123
  elif test $EC -eq 1; then 
    # error
    echo "Error!"
    return 1
  elif test $EC -eq 139; then
    # segmentation fault.
    CORE_FILE=`ls core* 2>/dev/null`
    if [ "$CORE_FILE" != "" ] ; then
      echo "======================= Core Dump Stack ==========================="
      gdb -ex where -ex quit $1 $CORE_FILE
      #rm -f $CORE_FILE
    fi
  else
    CORE_FILE=`ls core* 2>/dev/null`
    if [ "$CORE_FILE" != "" ] ; then
      echo "======================= Core Dump Stack ==========================="
      gdb -ex where -ex quit $1 $CORE_FILE
      #rm -f $CORE_FILE
    fi
    echo "Unknown exit code: $EC"
  fi
  return $EC
}

#
# safekill child proceses
# arguments:
#  $1: pid
#
KILL_TIMEOUT=15
export KILL_TIMEOUT
safekill() {
  #WPID=`/bin/kill -p $@`
  WPID="$@"
  export WPID

  echo "safekill $WPID, timeout: $KILL_TIMEOUT"

  # start timer
  (sleep $KILL_TIMEOUT; echo "[DEAD LOCK]; force to kill $WPID"; /bin/kill -9 $WPID; exit 1) &
  # record timer pid
  TPID=$!
  export TPID

  kill $WPID
  wait $WPID
  WEC=$?
 
  # kill timer process 
  kill $TPID 2>/dev/null
  wait $TPID 1>/dev/null 2>&1
  TRC=$?
  echo "timer process return $TRC"
  if [ $TRC -eq 1 ] ; then
    echo "force shutdown";
    # dump logs and exit
    #dump_logs
    #return 1
  fi

  if [ $WEC -ne 0 ] ; then
    echo "$1 return '$WRC'"
    #exit 1
  fi   

  return 0 
}


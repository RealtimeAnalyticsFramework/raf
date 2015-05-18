#!/bin/sh
#
# start idgs 
#

if [ "$IDGS_HOME" = "" ]; then
  IDGS_HOME=`dirname "$0"`/..
  cd $IDGS_HOME
  IDGS_HOME=`pwd`
  cd -
  export IDGS_HOME;
fi

# start node
$IDGS_HOME/bin/idgs "$@"


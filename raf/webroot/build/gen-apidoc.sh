#!/bin/bash
#
# script of build IDGS, called by Jenkins
#

# exit when failure
set +e

if [ "$WORKSPACE" == "" ]; then
  WORKSPACE=`pwd`
  WORKSPACE="$WORKSPACE/.."
  export WORKSPACE
fi
echo $WORKSPACE
cd $WORKSPACE/idgs

#
# generate API doc
#
echo "doxygen"
doxygen build/doxygen.conf 1>/dev/null 2>&1

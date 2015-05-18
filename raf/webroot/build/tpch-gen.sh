#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# TPCH_HOME: working dir of tpch-gen
# TPCH_SIZE: in GB

# uncompress tpch
if [ ! -d "$TPCH_HOME" ] ; then
  mkdir $TPCH_HOME
fi

cd $TPCH_HOME
if [ ! -d "dbgen" ] ; then
  tar -xzvf $WORKSPACE/idgs/thirdparties/tooling/tpch_2_14_3.tgz
fi

cd dbgen
if [ ! -f "makefile" ] ; then
  sed -e '1,$s/^DATABASE=/DATABASE = ORACLE/' \
      -e '1,$s/^MACHINE =/MACHINE = LINUX/'i \
      -e '1,$s/^WORKLOAD =/WORKLOAD = TPCH/' \
      -e '1,$s/^CC      =/CC    = gcc/' \
      makefile.suite > makefile
fi

make 1>/dev/null 2>&1

if [ "$TPCH_SIZE" == "" ]; then
  TPCH_SIZE=0.1
  export TPCH_SIZE
fi

echo TPCH_SIZE = $TPCH_SIZE GB

if [ ! -f "lineitem.tbl" ] ; then
  ./dbgen -vf -s $TPCH_SIZE
fi


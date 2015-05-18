#!/bin/bash
# Following system environment variables
# WORKSPACE: parent directory of idgs
# SSB_HOME: working dir of ssb-gen
# SSB_SIZW: in GB

# uncompress tpch
if [ ! -d "$SSB_HOME" ] ; then
  mkdir $SSB_HOME
fi

cd $SSB_HOME
if [ ! -d "ssb-dbgen-master" ] ; then
  unzip $WORKSPACE/idgs/thirdparties/tooling/ssb-dbgen.zip
fi

cd ssb-dbgen-master
# append header file 
if [ -f "driver.c" ] ; then
  var=`grep -c "#include <sys/types.h>" driver.c`
  if [ $var -eq 0 ] ; then
    sed -i 's/#include <unistd.h>/&\n#include <sys\/types.h>/' driver.c	  
  fi
fi

if [ -f "shared.h" ] ; then
  var=`grep -c "MAXAGG_LEN    10" shared.h`
  if [ $var -ne 0 ] ; then
    sed -i 's/MAXAGG_LEN    10/MAXAGG_LEN    20/' shared.h
  fi   
fi

# genernate makefile if not exists
if [ ! -f "makefile" ] ; then
  sed -e '1,$s/^DATABASE=/DATABASE = DB2/' \
      -e '1,$s/^MACHINE =/MACHINE = LINUX/' \
      -e '1,$s/^WORKLOAD =/WORKLOAD = SSBM/' \
      -e '1,$s/^CC      =/CC    = gcc/' \
      makefile.suite > makefile
 
fi

make 2>/dev/null

if [ ! -n "$SSB_LINEORDER_SIZE" ] ; then
  SSB_LINEORDER_SIZE=0.01
  export SSB_LINEORDER_SIZE
fi

if [ ! -n "$SSB_CUSTOMER_SIZE" ] ; then
  SSB_CUSTOMER_SIZE=0.1
  export SSB_CUSTOMER_SIZE
fi

if [ ! -n "$SSB_SUPPLIER_SIZE" ] ; then
  SSB_SUPPLIER_SIZE=0.1
  export SSB_SUPPLIER_SIZE
fi

if [ ! -n "$SSB_PART_SIZE" ] ; then
  SSB_PART_SIZE=0.1
  export SSB_PART_SIZE
fi

if [ ! -n "$SSB_DATE_SIZE" ] ; then
  SSB_DATE_SIZE=1
  export SSB_DATE_SIZE
fi

# for all tables
#./dbgen -vf -s $SSB_SIZE -T a

# for table customer.tbl
if [ ! -f "customer.tbl" ] ; then
  ./dbgen -vf -s $SSB_CUSTOMER_SIZE -T c
fi

# for table part.tbl
if [ ! -f "part.tbl" ] ; then
  ./dbgen -vf -s $SSB_PART_SIZE -T p
fi

# for table supplier.tbl
if [ ! -f "supplier.tbl" ] ; then
   ./dbgen -vf -s $SSB_SUPPLIER_SIZE -T s
fi

if [ ! -f "date.tbl" ] ; then
# for table date.tbl
  ./dbgen -vf -s $SSB_DATE_SIZE -T d
fi

# for table lineorder.tbl
if [ ! -f "lineorder.tbl" ] ; then
  ./dbgen -vf -s $SSB_LINEORDER_SIZE -T l
fi


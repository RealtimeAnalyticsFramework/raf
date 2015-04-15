#!/bin/bash

export idgs_group="jk_head"
export IDGS_HOME=$WORKSPACE/idgs/dist

# 
# debug build
#
BUILD_DIR=$WORKSPACE/gcc_debug
export BUILD_DIR

if test ! -d "$BUILD_DIR"  ; then
  mkdir $BUILD_DIR
fi

cd $BUILD_DIR
/bin/bash $WORKSPACE/idgs/build/build.sh || exit $?

export GTEST_COLOR=yes

# 
# UT, IT and PT
#
cd $WORKSPACE/idgs
mkdir it_log
/bin/bash $WORKSPACE/idgs/build/runtest.sh || exit $?
/bin/bash $WORKSPACE/idgs/build/it.sh      || exit $?
/bin/bash $WORKSPACE/idgs/build/tpch-it.sh || exit $?
/bin/bash $WORKSPACE/idgs/build/ssb-it.sh  || exit $?

#cd $BUILD_DIR
#make clean

#
# release build
#
BUILD_DIR=$WORKSPACE/gcc_release
export BUILD_DIR
if test ! -d "$BUILD_DIR"  ; then
  mkdir $BUILD_DIR
fi

CXXFLAGS="-g -O3 -DNDEBUG"
export CXXFLAGS

cd $BUILD_DIR
if test ! -f "Makefile"; then
  ../idgs/configure --prefix=$BUILD_DIR/dist 1>/dev/null
fi

max_make(){
  local CORES=`grep processor /proc/cpuinfo | wc -l`
  make -j $CORES -k $@
}


echo "make"
#max_make 1>/dev/null || make || exit $?
#make clean

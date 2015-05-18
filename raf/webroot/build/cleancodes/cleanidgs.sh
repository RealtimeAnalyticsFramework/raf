#!/bin/bash
export BUILD_FRONT=1

TMP_DIR=/run
SCRIPT_DIR=`dirname $0`
cd $SCRIPT_DIR
SCRIPT_DIR=`pwd`

cd $TMP_DIR
mkdir workspace
cd workspace
rm -rf idgs
git clone /var/git/idgs.git
cd idgs

export BUILD_FRONT=1

aclocal
autoconf
libtoolize -c --force
automake -a -c --force
./configure

rm framework/src/idgs_gch.h
rm framework/src/idgs_gch.h.gch
touch framework/src/idgs_gch.h
make -j8 -k
make || exit $!
SRC_DIR=`pwd`

IDGS_CXXFLAGS="-O3 -DNDEBUG -I. -I./test -I./src"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/framework/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/services/store/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/services/rddng/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/client/c/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/samples/tpc-svc/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${GTEST_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${PROTOBUF_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${TBB_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -D_GLIBCXX_USE_NANOSLEEP "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -std=c++11 "


export CXXFLAGS="$IDGS_CXXFLAGS"
export PCH_HEADER="idgs_gch.h"

# clean USELESS using namespace
find . -name "*.cpp" | xargs $SCRIPT_DIR/cleannamespace.sh


# clean USELESS include
SUBDIRS="framework services client samples integration_test "

# header file first
for DIR in $SUBDIRS; do
  find $DIR -name "*.h" | sed -e '/\.pb\./d' | xargs $SCRIPT_DIR/cleaninclude.sh
done

# source file
for DIR in $SUBDIRS; do
  find $DIR -name "*.cpp" | sed -e '/\.pb\./d' | xargs $SCRIPT_DIR/cleaninclude.sh
done


make -j -k
make


export idgs_group="jk_sync"
export IDGS_HOME=$WORKSPACE/idgs/dist

# 
# debug build
#
BUILD_DIR=$WORKSPACE/gcc_debug
export BUILD_DIR
export MVN_OPTIONS="-T 1.5C"

rm -f $WORKSPACE/idgs/*.txt

if test ! -d "$BUILD_DIR"  ; then
  mkdir $BUILD_DIR
fi

chmod 777 $WORKSPACE/idgs

cd $BUILD_DIR
export idgs_mtu=0
/bin/bash $WORKSPACE/idgs/build/build.sh        || exit $?

#LD_PRELOAD=/usr/local/lib/libtcmalloc.so
#export LD_PRELOAD

# 
# UT, IT and PT
#
export GTEST_COLOR=yes
cd $WORKSPACE/idgs
mkdir it_log
/bin/bash $WORKSPACE/idgs/build/runtest.sh
/bin/bash build/it.sh case51                 || exit $?

#echo "=============================== using ORDERED persist type ==============================="
#unset DEFAULT_PERSIST_TYPE
#/bin/bash $WORKSPACE/idgs/build/it.sh         || exit $?
#echo ""
#echo "=============================== using NONE persist type ==============================="
#export DEFAULT_PERSIST_TYPE=NONE
#/bin/bash $WORKSPACE/idgs/build/it.sh         || exit $?
#unset DEFAULT_PERSIST_TYPE
#/bin/bash $WORKSPACE/idgs/build/tpch-it.sh    || exit $?
#/bin/bash $WORKSPACE/idgs/build/ssb-it.sh     || exit $?


#cd $BUILD_DIR
#make clean

#
# release build
#
#rm -f $WORKSPACE/idgs/*.txt

BUILD_DIR=$WORKSPACE/gcc_release
export BUILD_DIR
export MVN_OPTIONS="-T 1.5C"

if test ! -d "$BUILD_DIR"  ; then
  mkdir $BUILD_DIR
fi

#LD_PRELOAD=/usr/local/lib/libtcmalloc.so
#export LD_PRELOAD
CXXFLAGS="-g -Ofast -DNDEBUG"
export CXXFLAGS

export GTEST_COLOR=yes

cd $BUILD_DIR
#/bin/bash $WORKSPACE/idgs/build/build.sh    || exit $?

export idgs_mtu=0

export TCMALLOC_MEMFS_MALLOC_PATH=/dev/shm

cd $WORKSPACE/idgs
#/bin/bash $WORKSPACE/idgs/build/tpch.sh     || exit $?
#/bin/bash $WORKSPACE/idgs/build/ssb.sh      || exit $?


# cd $BUILD_DIR
# make clean

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
if [ "$BUILD_DIR" == "" ]; then
  BUILD_DIR=$WORKSPACE/idgs
  export BUILD_DIR
fi

max_make(){
  local CORES=`grep processor /proc/cpuinfo | wc -l`
  make -j $CORES -k $@
}


# dump stdout and stderr when error.
# $1 executable
#
run_app() {
  echo "$@"
  $@ 1>1.log 2>&1
  RC_CODE=$?
  if [ $RC_CODE -ne 0 ] ; then
    cat 1.log
    rm -f 1.log
    exit $RC_CODE
  fi
  rm -f 1.log
}

# killall -9 idgs

#
# build
#
echo "#############################  Building ###################################"
echo "workspace: $WORKSPACE"
cd $WORKSPACE/idgs
mkdir it_log
if test ! -f "aclocal.m4" ; then
  run_app aclocal
fi
if test ! -f "configure" ; then
  run_app autoconf
fi
if test ! -f "ltmain.sh" ; then
  run_app libtoolize -c --force
fi
if test ! -f "Makefile.in" ; then
  run_app automake -a -c --force
fi

cd $BUILD_DIR
if test ! -f "Makefile"; then
  run_app $WORKSPACE/idgs/configure --prefix=$WORKSPACE/idgs/dist
fi

echo "make"
max_make 1>/dev/null || make || exit $?
echo "install"
make install 1>/dev/null 2>&1 || exit $?


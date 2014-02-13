#!/bin/bash
if test "$BUILD_DIR" = "" ; then
  BUILD_DIR=`pwd`
  export BUILD_DIR
fi
find . -name "core.*" -exec rm -f {} \;

make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 ||make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 || make -j 30 ||exit $?
make install 1>/dev/null 2>&1

build/runtest.sh

#run cluster config test with env
build/runtest-env.sh


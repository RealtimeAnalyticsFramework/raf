#!/bin/bash
if test "$BUILD_DIR" = "" ; then
  BUILD_DIR=`pwd`
  export BUILD_DIR
fi
find . -name "core*" -exec rm -f {} \;

cvs up -d -P
if test ! -f "aclocal.m4" ; then
  aclocal
fi
if test ! -f "configure" ; then
  autoconf
fi
if test ! -f "ltmain.sh" ; then
  libtoolize -c --force
fi
if test ! -f "Makefile.in" ; then
  automake -a -c --force
fi
if test ! -f "Makefile"; then
  ./configure --prefix=$BUILD_DIR/dist
fi

make -k -j 8 || make || exit $?
make install 1>/dev/null 2>&1

build/runtest.sh

#run cluster config test with env
#build/runtest-env.sh


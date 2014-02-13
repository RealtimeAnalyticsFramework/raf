dnl Process this file with autoconf to produce a configure script.


AC_PREREQ(2.59)
AC_INIT(idgs, 0.1)
AC_CONFIG_SRCDIR([framework/src/idgs/util/singleton.h])
AC_CONFIG_HEADER([framework/src/idgs/util/config.h])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()
AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_LIBTOOL

#checks for libraries.
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lcpg':
AC_CHECK_LIB([cpg], [main])
# FIXME: Replace `main' with a function in `-lglog':
AC_CHECK_LIB([glog], [main])
# FIXME: Replace `main' with a function in `-lgtest':
AC_CHECK_LIB([gtest], [main])
# FIXME: Replace `main' with a function in `-lgtest_main':
AC_CHECK_LIB([gtest_main], [main])
# FIXME: Replace `main' with a function in `-lprotobuf':
AC_CHECK_LIB([protobuf], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-ltbb':
AC_CHECK_LIB([tbb], [main])
# FIXME: Replace `main' with a function in `-lyajl':
AC_CHECK_LIB([yajl], [main])



# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gettimeofday memset mkdir select setenv socket sqrt])

echo "====================================================================="
echo " setup enviroment"
echo "$srcdir setup enviroment"
echo "====================================================================="
# IDGS sepcified variables
BUILD_DIR=`pwd`
SRC_DIR=$srcdir
cd $SRC_DIR
SRC_DIR=`pwd`
ac_default_prefix=$BUILD_DIR/dist
cd $BUILD_DIR

IDGS_CXXFLAGS="-I. -I./test -I./src -pthread"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/framework/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/services/store/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/services/rdd/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/services/admin/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/client/c/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I$SRC_DIR/samples/tpc-svc/src "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${GTEST_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${PROTOBUF_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -I${TBB_HOME}/include "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -D_GLIBCXX_USE_NANOSLEEP "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -std=c++11 "
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Wall -Wextra"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Werror=missing-prototypes"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Wno-ignored-qualifiers"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Wno-unused-parameter"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Wno-unused-variable"
IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Wno-sign-compare"
#IDGS_CXXFLAGS="$IDGS_CXXFLAGS -Werror"

# following line should be removed

# precompiled header
if test "$CXX" = "clang++" ; then
  IDGS_CXXFLAGS="$IDGS_CXXFLAGS -include idgs_gch.h "
fi

IDGS_LDFLAGS=""
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/framework "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/services/store "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/services/rdd "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/services/admin "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/client/c "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L$BUILD_DIR/samples/tpc-svc "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L${GTEST_HOME}/lib "
IDGS_LDFLAGS="$IDGS_LDFLAGS -L${PROTOBUF_HOME}/lib "
if test "$host_cpu" = "x86_64"  ; then
  IDGS_LDFLAGS="$IDGS_LDFLAGS -L${TBB_HOME}/lib/intel64/cc4.1.0_libc2.4_kernel2.6.16.21 "
else
  IDGS_LDFLAGS="$IDGS_LDFLAGS -L${TBB_HOME}/lib/ia32/cc4.1.0_libc2.4_kernel2.6.16.21 "
fi

# dump backtrace when C++ exception is thrown
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=33903
if test "$CXX" = "g++" ; then
IDGS_LDFLAGS="$IDGS_LDFLAGS -ldl -rdynamic -Wl,--wrap,__cxa_throw"
fi

IDGS_LDFLAGS="$IDGS_LDFLAGS -ltbb -ltbbmalloc -lprotobuf -lglog -L/usr/lib/$(uname -m)-linux-gnu -pthread -lcpg -lyajl "

itestdir='${exec_prefix}/itest'
utestdir='${exec_prefix}/utest'
confdir='${prefix}/conf'
pocdir='${prefix}/poc'
AC_SUBST(itestdir)
AC_SUBST(utestdir)
AC_SUBST(confdir)
AC_SUBST(pocdir)
AC_SUBST(IDGS_CXXFLAGS)
AC_SUBST(IDGS_LDFLAGS)
echo "IDGS_CXXFLAGS=$IDGS_CXXFLAGS"
echo "IDGS_LDFLAGS=$IDGS_LDFLAGS"
# IDGS end

#
# generate precompiled header
#
if test "$CXX" = "g++" ; then
  echo "====================================================================="
  echo " Generating GCH (GCC precompiled header)"
  echo "====================================================================="
elif test "$CXX" = "clang++" ; then
  echo "====================================================================="
  echo " Generating PCH ( clang precompiled header)"
  echo "====================================================================="
fi
$SRC_DIR/build/gen-gch.sh

#
# install google btree template library
#
if test -f $SRC_DIR/thirdparties/cpp-btree-1.0.1.tar.gz ; then
  tar -xzvf $SRC_DIR/thirdparties/cpp-btree-1.0.1.tar.gz
  rm cpp-btree-1.0.1/*test.h
  cp cpp-btree-1.0.1/*.h /usr/local/include
  rm -rf cpp-btree-1.0.1/
fi

AC_CONFIG_FILES(
  Makefile 
  framework/Makefile
  framework/src/Makefile
  services/Makefile
  services/store/Makefile
  services/store/src/Makefile
  services/rdd/Makefile
  services/rdd/src/Makefile
  services/admin/Makefile
  services/admin/src/Makefile  
  client/Makefile
  client/c/Makefile
  client/c/src/Makefile
  front_end/Makefile
  samples/Makefile
  samples/shell/Makefile
  samples/load/Makefile
  samples/tpc-svc/Makefile
  samples/allinone/Makefile
  integration_test/Makefile
  integration_test/rpc_it/Makefile
  integration_test/cluster_it/Makefile
  integration_test/store_it/Makefile
  integration_test/rdd_it/Makefile
  integration_test/client_it/Makefile
  integration_test/admin_it/Makefile  
)

AC_OUTPUT



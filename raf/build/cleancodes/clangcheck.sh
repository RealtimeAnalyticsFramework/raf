#!/bin/bash

WEBROOT=/media/sf_html
TMPROOT=/run

export PATH=$PATH:/media/sf_vmwork/sources/llvm_home/llvm/tools/clang/tools/scan-build

rm -rf $TMPROOT/workspace

prepare() {
  mkdir $TMPROOT/workspace
  cd $TMPROOT/workspace

  cvs co idgs
  cd idgs
  aclocal
  autoreconf
  autoconf
  automake
}

gccbuild() {
  cd $TMPROOT/workspace
  mkdir gccbuild
  cd gccbuild
  export CXX=g++
  scan-build --use-analyzer=/opt/llvm/bin/clang++ ../idgs/configure
  scan-build --use-analyzer=/opt/llvm/bin/clang++ -o $WEBROOT/llvm make -j4 -k
}

clangbuild() {
  cd $TMPROOT/workspace
  mkdir clangbuild
  cd clangbuild
  export CXX=clang++
  scan-build --use-analyzer=/opt/llvm/bin/clang++ ../idgs/configure
  scan-build --use-analyzer=/opt/llvm/bin/clang++ -o $WEBROOT/llvm make -j4 -k
}

prepare
gccbuild
#clangbuild


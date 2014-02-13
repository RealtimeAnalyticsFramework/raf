#!/bin/bash

WEBROOT=/media/sf_html
TMPROOT=/run

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
  scan-build --use-analyzer=/opt/llvm/bin/clang++ -o $WEBROOT/llvm make
}

clangbuild() {
  cd $TMPROOT/workspace
  mkdir clangbuild
  cd clangbuild
  export CXX=clang++
  scan-build --use-analyzer=/opt/llvm/bin/clang++ ../idgs/configure
  scan-build --use-analyzer=/opt/llvm/bin/clang++ -o $WEBROOT/llvm make
}

prepare
gccbuild
#clangbuild


#!/bin/bash
#
# generate GCH (precompiled header)
#

DIRNAME=`dirname $0`
cd $DIRNAME/..

if [ "$CXX" = "" ] ; then
  CXX=g++
fi

cd framework
cd src

find . -name "*.h" -exec grep "#include" {} \; | grep "<" >h1.txt
find . -name "*.cpp" -exec grep "#include" {} \; | grep "<" >h2.txt

cat h1.txt h2.txt | sort | uniq | sed -e '/yajl/i\
extern "C" {' -e '/yajl/a\
}'  >idgs_gch.h.new

rm -f h1.txt h2.txt

if diff idgs_gch.h idgs_gch.h.new; then
  rm -f idgs_gch.h.new
  echo "GCH: no change."
else
  mv idgs_gch.h.new idgs_gch.h
  # compile the precompiled header
  if test "$CXX" = "g++" ; then
    g++ -c -std=c++11 -I$TBB_HOME/include idgs_gch.h
  elif test "$CXX" = "clang++"; then
    echo "clang++ -I$TBB_HOME/include -std=c++11 -x c++-header idgs_gch.h -o idgs_gch.h.pch"
    clang++ -I$TBB_HOME/include -std=c++11 -x c++-header idgs_gch.h -o idgs_gch.h.pch
  else
    echo "Unknown c++ compiler $CXX"
  fi
fi

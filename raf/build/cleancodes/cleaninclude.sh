#!/bin/bash
# prune include files one at a time, recompile, and put them back if it doesn't compile
# arguments are list of files to check
# e.g. find . -name "*.h" | xargs cleaninclude.sh

# configuration 
# CXX: c++ compiler
# CXXFLAGS: compiler options
# PCH_HEADER: precompiled header
if [ "$CXX" = "" ] ; then
  CXX=g++
fi
if [ "$CXXFLAGS" = "" ] ; then
  CXXFLAGS="-std=c++11 -I."
fi
if [ "$PCH_HEADER" = "" ] ; then
  PCH_HEADER=""
fi

# yes or no
#CHECK_HEADER_COMPILABLE=no
CHECK_HEADER_COMPILABLE=no

#
# remove header file from source file
# $1 source file
# $2 header
removeinclude() {
    local file=$1
    local header=$2
    perl -i -p -e 's+([ \t]*#include[ \t][ \t]*[\"\<]'$header'[\"\>])+//REMOVEINCLUDE $1+' $file
}

#
# revert the source file
# $1 source file
replaceinclude() {
   local file=$1
   perl -i -p -e 's+//REMOVEINCLUDE ++' $file
}

#
# check whether source and header have the same name.
# $1: source file
# $2: header name
samename() {
  local f=`basename $1`
  local h=`basename $2`

  if [[ $f == *.cpp ]] ; then
    local f2=`echo $f | sed 's/cpp$//'`
    local h2=`echo $h | sed 's/h$//'`
    if [[ "$f2" == "$h2" ]] ; then
      echo "same name $f $h"
      return 0
    else
      return 1;
    fi
  else
    return 0
  fi 
}

#
# check whether the souce file compilable
# $1: source file
checkheader() {
   if test "x$CHECK_HEADER_COMPILABLE" = "xno"; then
     return 0
   fi
   local file=$1
   if [[ $file == *.cpp ]] ; then
     return 0
   fi 
   cat <<END >temptest.cpp
#include "$file"
void temp_test() {
}
END
  #echo $CXX $CXXFLAGS -c temptest.cpp -o temptest.o
  $CXX $CXXFLAGS -c temptest.cpp -o temptest.o 1>temptest.log 2>&1
  ECODE=$?
  rm -f temptest.cpp  temptest.o
  return $ECODE
} 

#
# main loop
#
for file in $*
do
   # compile the header file before change.
   if checkheader $file ; then
     XX=0
   else
     echo "========================================================"
     echo "#failed to compile $file"
     if [ $ECODE -ne 0 ] ; then
       cat temptest.log
     fi
     echo "========================================================"
     if test "x$CHECK_HEADER_COMPILABLE" = "xyes"; then
       continue
     fi
     #exit 1
   fi
       
    includes=`grep "^[ \t]*#include" $file | awk '{print $2;}' | sed 's/[\"\<\>]//g'`
    for i in $includes
    do
        if [ "$i" = "$PCH_HEADER" ] ; then
          continue
        fi
        echo "checking source: $file, header: $i"
        
        if samename $file $i ; then
          if [[ $file == *.cpp ]] ; then
            continue;
          fi
        fi

        touch $file # just to be sure it recompiles
        removeinclude $file $i

        if checkheader $file ; then
          XX=0
        else
          replaceinclude $file
          echo "failed to compile $file without $i"
          continue
        fi
	
	make -j8 -k >/dev/null 2>&1
        if make >/dev/null  2>&1;
        then
            grep -v REMOVEINCLUDE $file > tmp && mv tmp $file
            echo removed $i from $file
        else
            replaceinclude $file
            echo $i was needed in $file
        fi
    done
done

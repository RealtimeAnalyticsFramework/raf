#!/bin/bash

# 0->remove and re-install, default
# 1->update and install
flag=0

if [ $# -gt 0 ]; then
  flag=$1
fi  

IDGS_PARENT_HOME="/home"
IDGS_HOME="$IDGS_PARENT_HOME/idgs"
if [ ! -d $IDGS_PARENT_HOME ]; then
  mkdir -p $IDGS_PARENT_HOME
fi

if [ $flag -eq 0 ]; then 
# remove and re-install
  if [ -d $IDGS_HOME ]; then
    echo "-----------------remove $IDGS_HOME-------------"
    rm -rf $IDGS_HOME
  fi  
  echo "-----------------check out code from CVS-------------"
    cd $IDGS_PARENT_HOME
    cvs co idgs
else
# update install
  if [ ! -d $IDGS_HOME ]; then
# dir not exists, taking as re-install
    echo "-----------------check out code from CVS-------------"
    cd $IDGS_PARENT_HOME
    cvs co idgs
  else
    echo "-----------------update code from CVS-------------"
    cd $IDGS_HOME
    cvs update
  fi
fi

echo "-----------------compile and install code-------------"
cd $IDGS_HOME
if [ ! -f "Makefile" ]; then
  aclocal && autoreconf && automake && ./configure && sed -i 's/$echo/$ECHO/g' libtool && ./cicheck.sh
else
  ./cicheck.sh
fi

RC=$?
if [ $RC -ne 0 ]; then
  echo "-----------------install code error-------------"
  exit
fi
echo "-----------------install OK-------------"

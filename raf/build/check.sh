#!/bin/sh
#
# script to check IDGS, called by Jenkins
#
if [ "$WORKSPACE" == "" ]; then
  WORKSPACE=`pwd`
  WORKSPACE="$WORKSPACE/.."
  export WORKSPACE
fi
echo $WORKSPACE

cd $WORKSPACE/
cd $WORKSPACE/idgs

rm -f configure config.guess config.sub depcomp missing install-sh
find . -name "Makefile" -exec rm -f {} \; 2>/dev/null
find . -name "Makefile.in" -exec rm -f {} \; 2>/dev/null
find . -name "*.pb.cc" -exec rm -f {} \; 2>/dev/null
find . -name "*.pb.h" -exec rm -f {} \; 2>/dev/null
rm -f ltmain.sh 2>/dev/null
rm -rf dist/include

#
# sloc count
#
echo "#############################  sloccount  ###################################"
sloccount --wide --details . >sloccount.txt

#
# cppcheck
#
echo "#############################  cppcheck  ###################################"
cppcheck --quiet -v --inline-suppr --suppressions-list=build/cppcheck-suppress.txt  --xml --enable=all . 2>cppcheck_result.xml


#
# cccc 
#
echo "#############################  cccc  ###################################"
mkdir .cccc 2>/dev/null
find . -type f | grep -E '(\.cpp|\.h|\.cc|\.java)$' | xargs cccc --lang=c++ --db_infile=.cccc/cccc.db 2>cccc_error.txt 1>.cccc/out.txt


#
# cppncss
#
echo "#############################  cppncss  ###################################"
# top 50
/opt/cppncss-1.0.3/bin/cppncss -r -x -k -f=cppncss.xml .
# add XSLT header.
cat build/cppncss_head.txt cppncss.xml > cppncss_out.xml 

#
# statcvs
#
echo "#############################  statcvs  ###################################"
rm -rf statcvs 2>/dev/null
mkdir statcvs
cvs log>statcvs/cvs.log
java -jar thirdparties/tooling/statcvs.jar -include "**/*.cpp;**/*.h;**/*.proto;**/*.am;**/*.ac;**/*.conf;**/*.xml;**/*.txt;**/*.sh;*.xsl" -exclude "**/ltmain.sh;**/*.in;**/Makefile;" -verbose -output-dir statcvs -disable-twitter-button statcvs/cvs.log .




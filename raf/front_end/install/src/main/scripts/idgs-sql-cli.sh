#!/bin/bash
#
# IDGS SQL Command Line Interface.
#

if [ "$IDGS_HOME" = "" ]; then
  IDGS_HOME=`dirname "$0"`/..
  cd $IDGS_HOME
  IDGS_HOME=`pwd`
  cd -
  export IDGS_HOME;
fi

CLASSPATH=$IDGS_HOME:$IDGS_HOME/javalib/protobuf-java-2.6.1.jar
for JAR in `ls $IDGS_HOME/javalib`; do
  CLASSPATH=$CLASSPATH:$IDGS_HOME/javalib/$JAR
done

JAVA=java

#echo $CLASSPATH

#JAVA_OPTS="-cp $CLASSPATH -server -Xms1g -Xmx2g -Dcom.sun.management.jmxremote"
JAVA_OPTS="-cp $CLASSPATH "
JAVA_MAINCLASS="idgs.IdgsCliDriver"

# start client
$JAVA $JAVA_OPTS $JAVA_MAINCLASS "$@"

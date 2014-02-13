#!/bin/bash
# run sql_engine

if [ "$SQL_ENGINE_HOME" = "" ]; then
  SQL_ENGINE_HOME=`dirname "$0"`/..
  export SQL_ENGINE_HOME;
fi

PROTOBUF_JAR=$SQL_ENGINE_HOME/lib/protobuf-java-2.5.0.jar

CLASSPATH=$SQL_ENGINE_HOME
CLASSPATH=$CLASSPATH:$PROTOBUF_JAR
for JAR in `ls $SQL_ENGINE_HOME/lib`; do
  if [ $JAR != $PROTOBUF_JAR ]; then
    CLASSPATH=$CLASSPATH:$SQL_ENGINE_HOME/lib/$JAR
  fi
done

JAVA=java

JAVA_OPTS="-cp $CLASSPATH -server -Xms1g -Xmx2g -Dcom.sun.management.jmxremote"
JAVA_MAINCLASS="idgs.IdgsCliDriver"
APP_OPTS="$@"

# start client

$JAVA $JAVA_OPTS $JAVA_MAINCLASS $APP_OPTS
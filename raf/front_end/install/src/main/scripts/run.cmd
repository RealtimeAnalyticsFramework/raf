echo off

set SQL_ENGINE_HOME=../
set CLASSPATH=.;..;..\lib\protobuf-java-2.5.0.jar;lib\*

rem export JAVA_HOME=""
set JAVA=java

set JAVA_OPTS=-cp %CLASSPATH% -server -Xms512m -Xmx512m -Dcom.sun.management.jmxremote
set JAVA_MAINCLASS=idgs.IdgsCliDriver
set APP_OPTS=

echo on

%JAVA% %JAVA_OPTS% %JAVA_MAINCLASS% %APP_OPTS%
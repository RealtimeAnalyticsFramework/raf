#!/bin/bash

INSTALL_DIR="/home"
INSTALL_SCRIPT_FILE="local-install.sh"

SERV_LIST="10.10.10.53 \
	   10.10.10.55"

#copy install file to servers
for host in $SERV_LIST
do
  echo "------------------copy install file to server $host-------------------"
  scp ./$INSTALL_SCRIPT_FILE root@$host:$INSTALL_DIR
  RC=$?
  if [ $RC -ne 0 ]; then
    echo "------------------copy install file to server $host error-------------------"
    exit
  fi
done

for host in $SERV_LIST
do
  echo "------------------compile and install code on server: $host-------------------"
  scp ./$INSTALL_SCRIPT_FILE root@$host:$INSTALL_DIR
  ssh $host "source /etc/profile; cd /home; ./local-install.sh" &
done

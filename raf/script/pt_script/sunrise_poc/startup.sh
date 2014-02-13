#!/bin/bash
ensure_corosync(){
  pid=`pgrep corosync`
  if [ -z $pid ] ; then
  	echo "starting corosync..."
  	corosync 1>/dev/null 2>&1
  	sleep 1
  else
  	echo "corosync has been started..."	
  fi	
}

# OS tuning
sysctl -w net.core.rmem_max=209630400
sysctl -w net.core.rmem_default=209630400
sysctl -w net.core.wmem_max=209630400
sysctl -w net.core.wmem_default=209630400
sysctl -w net.core.optmem_max=20480000
sysctl -w net.core.netdev_max_backlog=100000

#start corosync
ensure_corosync

# enable tcmalloc if exists
if [ -f /usr/lib/libtcmalloc.so ] ; then
  export LD_PRELOAD=/usr/lib/libtcmalloc.so
  echo "tcmalloc is enabled..."
elif [ -f /usr/local/lib/libtcmalloc.so ] ; then
  export LD_PRELOAD=/usr/local/lib/libtcmalloc.so  
  echo "tcmalloc is enabled..."
fi

# numbers of cpu processors
processors=`cat /proc/cpuinfo |grep processor |cut -f2 -d ':'|wc -l`
echo "cpu processors: $processors"

# set work thread count
export idgs_thread_count=100
echo "work thread count: $idgs_thread_count"

# set coredump 
ulimit -c unlimited

# set max socket connections linux default(1024)
ulimit -n 10240

# start member(filename like *transformer, loglevel=2)
GLOG_vmodule=main=1,*transformer=0,*rdd*=0 dist/bin/idgs-aio -c poc/sunrise/conf/cluster.conf 1>idgs_sunrise_serv_$$.log 2>&1 &
SERV_ID=$$
sleep 3
tail -f idgs_sunrise_serv_$SERV_ID.log

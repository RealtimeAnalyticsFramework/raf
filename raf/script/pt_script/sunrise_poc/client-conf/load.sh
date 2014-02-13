#!/bin/bash

# OS tuning
sysctl -w net.core.rmem_max=209630400
sysctl -w net.core.rmem_default=209630400
sysctl -w net.core.wmem_max=209630400
sysctl -w net.core.wmem_default=209630400
sysctl -w net.core.optmem_max=20480000
sysctl -w net.core.netdev_max_backlog=100000

# enable tcmalloc if exists
if [ -f /usr/lib/libtcmalloc.so ] ; then
  export LD_PRELOAD=/usr/lib/libtcmalloc.so
  echo "tcmalloc is enabled..."
elif [ -f /usr/local/lib/libtcmalloc.so ] ; then
  export LD_PRELOAD=/usr/local/lib/libtcmalloc.so  
  echo "tcmalloc is enabled..."
fi

# set coredump 
ulimit -c unlimited

# modify max socket connections, linux default(1024)
ulimit -n 10240

# load files directory

LOAD_FILES_DIR=/home/sunrise_poc/

if [ "$LOAD_FILES_DIR" == "" ]; then
	export LOAD_FILES_DIR="/dev/shm"
else
	echo "LOAD_FILES_DIR: $LOAD_FILES_DIR"
fi

MULTIPLE_OF_LOAD_SRC_FILE=3000

# mutiple of source file, default 30x (~100M)
if [ "$MULTIPLE_OF_LOAD_SRC_FILE" == "" ]; then
	export MULTIPLE_OF_LOAD_SRC_FILE=30
else
	echo "MULTIPLE_OF_LOAD_SRC_FILE: $MULTIPLE_OF_LOAD_SRC_FILE"
fi


# batch lines number of loader
if [ "$BATCH_INSERT_LINES" == "" ]; then
	export BATCH_INSERT_LINES=1000
fi

# timeout default 60s
if [ "$BATCH_INSERT_TIME_OUT" == "" ]; then
	export BATCH_INSERT_TIME_OUT=100
fi

gen_data() {
	echo "#######################gen data#######################"
	# source file name
	export LOAD_SRC_FILE="poc/sunrise/data_std.dat"
	if [ ! -f "$LOAD_SRC_FILE" ]; then
		echo "Gen data error, source file: $LOAD_SRC_FILE not found" 
		exit;
	fi
	SRC_FILE_SIZE=`du -h $LOAD_SRC_FILE |awk '{print $1}'`
	SRC_FILE_COUNT=`wc -l $LOAD_SRC_FILE |awk '{print $1}'`
	echo "Src File: $LOAD_SRC_FILE $SRC_FILE_SIZE $SRC_FILE_COUNT record(s)"
	echo "Gen dest file($MULTIPLE_OF_LOAD_SRC_FILE x)..."
	DST_FILE_PATH="$LOAD_FILES_DIR/data_std.dat"
	awk 'BEGIN{FS="|"; OFS="|"} {a=$0; for(i=0; i < n; ++i) {b=$2; $2=$2i; a=a"\n"$0; $2=b;} print a}' n=$MULTIPLE_OF_LOAD_SRC_FILE $LOAD_SRC_FILE > $DST_FILE_PATH
	DST_FILE_SIZE=`du -h $DST_FILE_PATH |awk '{print $1}'`
	DST_FILE_COUNT=`wc -l $DST_FILE_PATH |awk '{print $1}'`
	echo "Dest File: $DST_FILE_PATH $DST_FILE_SIZE $DST_FILE_COUNT record(s)"
	echo "#######################//gen data#######################"
}

# load thread count, default 50
if [ "$LOAD_THREAD_COUNT" == "" ]; then
	export LOAD_THREAD_COUNT=50
fi

echo "LOAD_THREAD_COUNT: $LOAD_THREAD_COUNT"

singleline_load() { 
	echo "#######################standalone loader#######################"
  	time dist/bin/load -s 0 -p $LOAD_FILES_DIR -c poc/sunrise/conf/client.conf -m poc/sunrise/conf/sunrise_file_mapper.conf -t $LOAD_THREAD_COUNT -o sunrise_batch_line_load_tps.txt
}

incluster_load() {
  	echo "#######################in cluster loader#######################"
  	export idgs_member_port=18800
  	export idgs_member_innerPort=18801
  	export idgs_member_service_local_store=false
  	time dist/bin/load -s 1 -p $LOAD_FILES_DIR -c poc/sunrise/conf/cluster.conf -m poc/sunrise/conf/sunrise_file_mapper.conf -t $LOAD_THREAD_COUNT -o sunrise_incluster_load_tps.txt
}

batchline_load() { 
	echo "#######################batch line loader#######################"
  	time dist/bin/load -s 2 -p $LOAD_FILES_DIR -c poc/sunrise/conf/client.conf -m poc/sunrise/conf/sunrise_file_mapper.conf -t $LOAD_THREAD_COUNT -o sunrise_batch_line_load_tps.txt
  	echo "BATCH_INSERT_TIME_OUT: $BATCH_INSERT_TIME_OUT"
  	echo "BATCH_INSERT_LINES: $BATCH_INSERT_LINES"
}

# gen load data
LOAD_FILE="$LOAD_FILES_DIR/data_std.dat"
if [ ! -f "$LOAD_FILE" ]; then
	gen_data
else
	echo "Gen dest file: $LOAD_FILE already exists"
fi

LOAD_FILE_SIZE=`du -h $LOAD_FILE |awk '{print $1}'`
LOAD_FILE_ROW_COUNT=`wc -l $LOAD_FILE |awk '{print $1}'`
echo "===============load file==============="
echo "$LOAD_FILE , size: $LOAD_FILE_SIZE , row count: $LOAD_FILE_ROW_COUNT"
echo "=============//load file==============="

# default using batchline_loader(2)
which_loader=2;
if [ $# == 1 ]; then
	which_loader=$1;
fi

batchline_load
#singleline_load

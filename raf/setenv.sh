# $Header: /var/cvsroot/idgs/setenv.sh,v 1.5 2013/11/07 07:24:53 tinglou Exp $
# setenv for idgs
# Usage: 
# . ./setenv.sh
#

HOST=`hostname`
echo "Hostname: " $HOST

CPUS=`cat /proc/cpuinfo | grep processor | wc -l`
echo "CPU cores: " $CPUS

# tcmalloc
# http://gperftools.googlecode.com/svn/trunk/doc/tcmalloc.html
# TCMALLOC_MEMFS_MALLOC_PATH	default: ""	 If set, specify a path where hugetlbfs or tmpfs is mounted. This may allow for speedier allocations.

# public address
export idgs_member_ip=$HOST

# public port
export idgs_member_port=7700

# inner address
export idgs_member_innerIp=$HOST

# inner port
export idgs_member_innerPort=8800

# worker thread pool
export idgs_thread_count=10

# IO thread pool
export idgs_io_thread_count=4

# idle thread may save energy.
export idgs_max_idle_thread=1

# MTU of UDP channel, MTU=0 means TCP only  
export idgs_mtu=0

# Innter TCP: batch recv/send messages count. Larger batch size, less system call.
export idgs_tcp_batch=200

# partition count, should be a primary integer, it is recommended to be <total CPU core> X 2
export idgs_partition_count=233

# whether local member store data.
export idgs_member_service_local_store=true

# GLOG related
export GLOG_v=0
export GLOG_vmodule="main=1,rdd*=0"

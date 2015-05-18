# $Header: /var/cvsroot/idgs/setenv.sh,v 1.7 2015/02/06 06:43:52 xitang Exp $
# setenv for idgs
# Usage: 
# . ./setenv.sh
#

IS_NOT_SET() {
   [[ -n $1 ]] && return 1 || return 0
}

HOST=`hostname`
echo "Hostname: " $HOST

CPUS=`cat /proc/cpuinfo | grep processor | wc -l`
echo "CPU cores: " $CPUS

# tcmalloc
# http://gperftools.googlecode.com/svn/trunk/doc/tcmalloc.html
# TCMALLOC_MEMFS_MALLOC_PATH    default: ""      If set, specify a path where hugetlbfs or tmpfs is mounted. This may allow for speedier allocations.

# public address
if IS_NOT_SET $idgs_public_host
  then export idgs_public_host=$HOST 
  else echo idgs_public_host=$idgs_public_host  
fi

# public port
if IS_NOT_SET $idgs_public_port
  then export idgs_public_port=7700
  else echo idgs_public_port=$idgs_public_port
fi

# inner address
if IS_NOT_SET $idgs_inner_host
  then export idgs_inner_host=$idgs_public_host
  else echo idgs_inner_host=$idgs_inner_host
fi

# inner port
if IS_NOT_SET $idgs_inner_port
  then export idgs_inner_port=8800
  else echo idgs_inner_port=$idgs_inner_port 
fi

# worker thread pool
if IS_NOT_SET $idgs_thread_count
  then export idgs_thread_count=10
  else echo idgs_thread_count=$idgs_thread_count  
fi

# IO thread pool
if IS_NOT_SET $idgs_io_thread_count
  then export idgs_io_thread_count=4
  else echo idgs_io_thread_count=$idgs_io_thread_count  
fi

# idle thread may save energy.
if IS_NOT_SET $idgs_max_idle_thread
  then export idgs_max_idle_thread=1
  else echo idgs_max_idle_thread=$idgs_max_idle_thread  
fi

# MTU of UDP channel, MTU=0 means TCP only
if IS_NOT_SET $idgs_mtu
  then export idgs_mtu=0
  else echo idgs_mtu=$idgs_mtu   
fi

# Innter TCP: batch recv/send messages count. Larger batch size, less system call.
if IS_NOT_SET $idgs_tcp_batch
  then export idgs_tcp_batch=200
  else echo idgs_tcp_batch=$idgs_tcp_batch   
fi

# partition count, should be a primary integer, it is recommended to be <total CPU core> X 2
if IS_NOT_SET $idgs_partition_count
  then export idgs_partition_count=233
  else echo idgs_partition_count=$idgs_partition_count  
fi

# whether local member store data.
if IS_NOT_SET $idgs_local_store
  then export idgs_local_store=true
  else echo idgs_local_store=$idgs_local_store  
fi

# GLOG related
if IS_NOT_SET $GLOG_v  
  then export GLOG_v=0
  else echo GLOG_v=$GLOG_v
fi



[TOC]

# 集群配置 #
IDGS基于Linux的集群管理框架Corosync来管理自己的集群。Corosync有很多服务，目前只用了cpg服务。在启动idgs前，必须在每个机器正确配置corosync，并且启动corosync守护进程。

Corosync默认配置文件位于/etc/corosync/corosync.conf，采用类似JSON的格式，其中必须根据网络实际情况修改的属性有：

- **bindnetaddr**
> 该机器的网络地址，如果集群所有机器都处于同一子网络，此配置项可以相同。如果该机器有多块网卡，且处于同一子网，必须指定特定地址。

- **mcastaddr**，**mcastport**
> corosync基于UDP组播，Mcastaddr必须是合法的组播地址；Mcastaddr和mcastport的组合必须和其他组播服务不冲突，如果有两个corosync集群，也必须变换其中某个值；集群中所有机器这两个值必须相同。

- **ttl**
>指定组播包传输的距离，单机为0，1个交换机内为1，2个交换机内为2，以此类推。一般不要超过4，而且途径的路由器或交换机必须适当配置支持组播。

举例说明如下：

	# Please read the corosync.conf.5 manual page
	totem {
        version: 2
        crypto_cipher: none
        crypto_hash: none

        interface {
                # Rings must be consecutively numbered, starting at 0.
                ringnumber: 0
                bindnetaddr: 192.168.1.0
                mcastaddr: 239.255.1.1
                mcastport: 5405
                ttl: 1
        }
		logging {
	        fileline: off
	        to_stderr: no
	        to_logfile: yes
	        logfile: /var/log/cluster/corosync.log
	        to_syslog: yes
	        debug: off
	        timestamp: on
	        logger_subsys {
	                subsys: QUORUM
	                debug: off
	        }
		}
	}

正确配置好corosync后，可以尝试启动corosync守护进程。

执行`corosync`或`service corosync start`，然后，用下述命令验证corosync是否启动成功

	ps  -ef | grep corosync

如进程存在则证明corosync启动成功。

# 服务器节点配置 #

## 配置文件 ##

IDGS的配置文件采用JSON格式，除了少数属性是集群通用的（所有机器上的值必须一致），其他的都是针对本机的环境定制的。重要属性说明如下,

- **partition\_count**
>分区数量，最好为集群中CPU总核数的若干倍，一般为质数。集群通用。

- **max\_replica\_count**
> 数据最多备份数，集群通用。 每个store（table）可以指定自己的备份个数，但是不能超过集群最多备份个数。集群通用。

- **modules**
> 节点支持的扩展服务，其中 `module_path` 和 `config_path` 是对应插件和配置文件的相对路径。集群内部的配置最好一致，当然文件路径可能有所不同。

- **member**
> 该节点的配置信息，其中：
>  - **public_address** 客户端连接地址，支持TCP/HTTP，传输内容为protobuf或JSON
>  - **inner_address** 集群内部通讯地址，支持TCP/UDP，采用内部通讯协议


示例配置文件如下：

    	{
    	  "thread_count":"3",
    	  "partition_count":"17",
          "group_name":"g1",
    	  "modules":[
	    	{
	    	  "name":"store",
	    	  "module_path":"dist/lib/libidgsdatastore.so",
	    	  "config_path":"services/store/test/data_store.conf"
	    	},
	    	{
	    	  "name":"rdd",
	    	  "module_path":"dist/lib/libidgsrddng.so"
	    	}
    	  ],
    	  "member": {
	    	"public_address": {
	    	  "host":"127.0.0.1",
	    	  "port":7700
	    	},
	    	"inner_address": {
	    	  "host":"127.0.0.1",
	    	  "port":7701
	    	},
	    	"service" : {
	    	}
    	  }
    	}
    
## 环境变量 ##
为了简化配置文件，方便配置管理，可以集群共享相同的配置文件，然后用shell脚本获取本机配置来设置相应环境变量。 环境变量可以覆盖配置文件的值。

程序启动时生效的配置优先级为： 环境变量 > 配置文件 > 默认值。

以下是常见环境变量列表：

	| 环境变量                     | 类型     | 对应配置项                          | 说明                                            |
	| --------------------------- | -------- | ----------------------------------| ----------------------------------------------- |
	| idgs_group                  | string   | group_name                        | 集群名称                                         |
	| idgs_partition_count        | uint32   | partition_count                   | 逻辑分区总数，和集群中CPU总核数相关                 |
	| idgs_weight                 | uint32   | member.weight                     | 该节点的权重                                     |
	| idgs_thread_count           | uint32   | thread_count                      | 工作线程数                                       |
	| idgs_io_thread_count        | uint32   | io_thread_count                   | IO线程数                                         |
	| idgs_max_idle_thread        | uint32   | max_idle_thread                   | 最大空闲线程数                                    |
	| idgs_mtu                    | uint32   | mtu                               | MTU                                             |
	| idgs_batch_message          | uint32   | batch_message                     | 内部通讯中批量传输的最大消息数，参见 batch_buffer    |
	| idgs_batch_buffer           | uint32   | batch_buffer                      | 内部通讯中批量传输的最大字节数，参见 batch_message   |
	| idgs_public_host            | string   | member.public_address.host        | 节点供外部访问的地址，IP或主机名                    |
	| idgs_public_port            | uint32   | member.public_address.port        | 节点供外部访问的端口                               |
	| idgs_inner_host             | string   | member.inner_address.host         | 节点供内部访问的地址，IP或主机名                    |
	| idgs_inner_port             | uint32   | member.inner_address.port         | 节点供内部访问的端口                               |
	| idgs_local_store            | boolean  | member.service.local_store        | 该节点是否存储数据                                 |
	| idgs_module_<mod-name>      | boolean  | member.service.<mod-name>.enable  | 该节点上指定服务是否可用                            |

## 服务器启动 ##
IDGS的最佳是使用方式是作为动态链接库，嵌入到用户程序内；
用户程序负责启动IDGS的初始化、启动和停止的生命周期，IDGS负责分区管理、集群RPC；
用户程序实现IDGS的插件来扩展功能。

为了方便使用，也提供了一个简单的测试应用`idgs`。
 `idgs -c <path-to-cluster-config>`

# 客户端 #
## 客户端配置文件 ##
客户端配置文件为JSON格式，所有客户程序通用。

重要属性说明如下，

- **server_addresses** 
> 服务器列表，客户端会尝试连接可用服务器，保持长连接。

- **pool_size** 
> 连接池大小。

示例配置文件如下：

		{
		  "server_addresses":
		    [
		      {
		        "host":"127.0.0.1",
		        "port":"7700"
		      },
		      {
		        "host":"127.0.0.1",
		        "port":"7701"
		      }
		    ],
		  "pool_size":"10",
		  "modules": [
		    {
		      "name":"store",
		      "config_file":"services/store/test/data_store.conf"
		    }    	
		  ]
		}

## 命令行接口 ##
提供一个简单的命令行工具来调用服务器功能`idgs-cli`。

### 命令行参数 ###
`idgs-cli [options]` 其中选项可以是：

* **-c [path-to-client-config]**
* **--client-config [path-to-client-config]**

> 指定客户端配置文件

* **-s [path-to-client-store-config]**
* **--store-config [path-to-client-store-config]**

> 指定客户端数据库配置文件

* **-f [path-to-script-file]**
* **--script_file [path-to-script-file]**

> 指定脚本文件，文件中可包含多条命令，如果没有指定脚本文件，则从标准输入中接受命令。命令的格式可参考如下。

* **-h**
* **--help**

> 显示帮助信息

### 命令语法 ###

	COMMAND :== SVC_NAME OP_NAME PARAM EXA_PARAM* ‘;’
	SVC_NAME :== ID
	OP_NAME :== ID 
	PARAM :== JavaScript Object Notation (JSON)
	EXA_PARAM :== ID ‘=’ PARAM
	ID :== [a-zA-Z0-9_-\.]+

### 示例脚本文件 ###


		# insert into Customer(partition_table)
		store.service insert {"store_name":"Customer"} key={"c_custkey":"234000"} value={"c_name":"Tom0","c_nationkey":"10","c_phone":"13500000000"};
		store.service insert {"store_name":"Customer"} key={"c_custkey":"234001"} value={"c_name":"Tom1","c_nationkey":"11","c_phone":"13500000001"};

其中，

* 服务：store.service
* 操作：insert 
* 参数：{"store_name":"Customer"}，指定插入的表名。 
* 扩展参数有两个，分别是key和value。

## CSV加载工具 ##
用来加载CSV文本文件到内存`idgs-load`。
TODO

# SQL支持 #
IDGS基于Apache Hive 1.0，实现了一个SQL引擎。

## SQL 命令行接口 ##
系统提供了一个基本兼容apache hive的命令行接口`idgs-sql-cli.sh`。

## JDBC 驱动 ##



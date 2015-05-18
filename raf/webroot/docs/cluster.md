##Cluster Management

从逻辑视图的角度来看，RAF集群是由多个RAF实例（RAF Member）组成的。这些实例之间需要通信，包括多播和单播通信。另外整个集群系统需要维护一个统一的状态，以达到稳定和可靠的需求。这些功能都是由ClusterAdapter和MembershipManager实现的。

从数据集的角度来看，整个RAF集群将数据进行分区（partition）存储，已达到高并发和高吞吐的要求。这样每个Member只存储数据集的1/n（n是集群中Member的数目）。Partition的维护工作是由PartitiontableManager完成的。

### ClusterAdapter

每个Member启动的时候都需要通知集群自己请求加入，加入后就可以接受和发送集群广播的消息（单播发送消息在Net章节有描述）。当前RAF的底层集群管理依赖于[Corosync](http://corosync.github.io/corosync/)。Coroysnc封装好了底层的集群多播通信协议，并且保证消息可达性和顺序性。
为了将底层的集群管理和传输协议与上层逻辑解耦，ClusterAdapter用来屏蔽底层的具体实现细节。

ClusterAdapter主要有如下两个接口:

* start -- join集群
* multicastMessage -- 广播消息

CorosyncClusterAdapter是ClusterAdpater的子类，负责接收Corosync的消息。
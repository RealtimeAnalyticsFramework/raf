[TOC]

## 利用Docker搭建RAF的开发与运行环境 ##

拷贝idgs/docker目录下的所有文件到自己的工作目录，在工作目录下作如下的操作

*下面所述的Docker主机指的是能够运行docker命令的环境*

### 1.Docker主机的设置 ###

#### 1.1 Proxy的设置
*如果Docker主机不是通过http proxy接入互联网的，请忽略这一节。*

在一些企业环境中，Docker的主机需要通过http proxy接入互联网。这时候需要通过如下配置才能使Docker Container接入互联网

-  配置主机的NetworkManager.conf

		vi /etc/NetworkManager/NetworkManager.conf 

修改:dns=dnsmasq 变成 #dns=dnsmasq

- 重启NetworkManager和docker

		service docker stop
		restart network-manager

- 通过加入HTTP Proxy参数启动Docker daemon

		HTTP_PROXY=http://proxy-xxxx.com:912/ docker -d &

### 2. 下载和编译第三方软件 ###

RAF依赖很多第三方库来编译或者运行RAF实例。通过查看Docerfile文件可以了解依赖的第三方库。

*如果需要通过Proxy接入互联网的话，需要修改Dockerfile里面的proxy主机名*

运行如下命令可以下载和编译第三方库：

		docker build -t raf_env - < Dockerfile

这个过程会持续的时间比较长。最终会创建一个名为raf_env的Docker images,可以通过运行`docker images`看到

### 3. 设置RAF的开发环境 ###

*如果需要通过Proxy接入互联网的话，需要修改Dockerfile_dev里面的proxy主机名*

运行如下命令可以创建一个RAF的开发环境

		docker build -t raf_dev - < Dockerfile_dev

*如果在第二步最后没有用raf__env,而是用其他为名创建docker image的话， 需要在Dockerfile_dev里修改第一行`FROM raf_env`，将`raf_env`替换为你取得名字*

#### 3.1 配置版本控制软件 ####

通过Dockerfile创建的image自带了CVS，下面以CVS为例来说明如何配置版本控制的环境

- `docker run -i -t raf_dev /bin/bash` 运行一个raf_dev的container
-  在刚才运行的container内

		echo "export CVSROOT=\":pserver:your_name@your_cvs_server_ip:2401/var/cvsroot\"" >> /etc/profile 
		source /etc/profile
		cvs login

- 然后就可以通过`cvs co idgs`来下载代码到container内了
- 下载完成后运行`exit`退出当前container
- 在Docker主机运行`docker ps -l`查看刚才运行的container id, 例如：*61e421bdf1d8*                      
- 在Docker主机运行`docker commit 61e421bdf1d8 "my_raf_dev"`

#### 3.2 配置可运行Eclipse的开发环境 ####

因为Eclipse需要通过GUI才能被用来做开发环境的IDE，所以需要在Docker主机和Docker Container才能在Docker主机看到Docker Container运行的Eclipse GUI

- 在Docker主机上运行：

		xhost +
		XSOCK=/tmp/.X11-unix/X0

- 最后通过运行如下命令就可以看到container内运行的Eclipse

		docker run -v $XSOCK:$XSOCK my_raf_dev eclipse

- 如果你想在另一个终端进入这个运行开发环境的container，你可以运行如下命令：

		docker exec -it [container id] /bin/bash



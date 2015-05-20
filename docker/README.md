## Introduction
This is the guide for the user to build docker images of RAF. For the user who is the new to docker, please refer this [guide](https://docs.docker.com/userguide/).

##Build RAF base images
RAF base image is the enviroment that has all the dependcy libraries been built in. It is basic for the compile and development
enviroment.

First, you must check if your host machine has installed openssh server, if not you must do the next step in
your host machine:

* install openssh-server, if the host machine is Ubuntu, please run `apt-get install openssh-server`
* By default, the SSH server denies password-based login for root. In /etc/ssh/sshd_config, change:
    `PermitRootLogin without-password` 
    to
    `PermitRootLogin yes`
    And restart SSH:
    `sudo service ssh restart`
* shutdown the firewall

To build the RAF base images, please run `./build_raf_base_images`

 * Note: Before you run this script, you need change the content in the  docker_base/Dockerfile as below:
  * The password and ip adress in the line `RUN sshpass  -p "intel@123"  scp -o StrictHostKeyChecking=no ~/.ssh/id_rsa.pub root@10.10.10.201:~/.ssh/authorized_keys`
     must be change to your host root password and ip address
  * You must remove the proxy in the `docker_base/Dockerfile` if you don't use the proxy server or you must change the
   proxy server name in the docker_base/Dockerfile if you use another proxy.

After the base images is built, you can use `docker images` command to see the `raf_base` image. Now you can run a container
from `raf_base` images. In this container you can use any source version controll software to download the RAF code or you can
use next step to download and build RAF

##Buld RAF compile images
After the base image is built, you can run ./build_raf_compiled_images to build the RAF compile images. This images will download
the source code from [RAF Git](https://github.com/RealtimeAnalyticsFramework/raf.git) and build the RAF source code. After the image
has been built, you can use `docker images` command to see the `raf_compiled` images is in your image repository.

 * Note: the `docker_compile/settings.xml` is the configuration for the maven, which will be copied into the image, you must
   change the proxy name in the settings.xml file or you must remove the proxy if you don't use proxy server. Also
   `docker_compile/Dockerfile` contains proxy setting, you must do the same action for this file.

##Run RAF standalone instance
After the raf_compiled image is created, you can start the images `docker run -i -t raf_compiled /bin/bash`. After the image
is tarted, you must run `corosync` cmd to start the corosync service, and then you can start one RAF instance by cmd: 
`dist/bin/start-idgs.sh` in the directory /home/project/idgs. 

##Running Exmaple
After running a RAF standalone instance, you can access the shell of the running docker container by `docker exec -i -t 665b4a1e17b6 bash
` cmd, the 665b4a1e17b6  is an example container id, you must change it to your running container in which the RAF instance is running

 * idgs-cli: you do CRUD operation on a specific store by this command shell
   * start idgs-cli: `dist/bin/idgs-cli`
   * insert an data into store LineItem: 
     ```javascript 
    store.service insert {"store_name":"LineItem"} key={"l_orderkey":"100000","l_linenumber":"1"} value={"l_partkey":"1","l_suppkey":"1","l_extendedprice":"203.55","l_discount":"0.01"};
    ```
   * get a data from store LineItem:
    ```javascript
    store.service get {"store_name":"LineItem"} key={"l_orderkey":"100000","l_linenumber":"1"};
    ```
 * idgs-sql: this shell gives you the right to query data by Hive like SQL. 
    * start idgs-sql: `dist/bin/idgs-sql-cli.sh`
    * you can query the store LineItem by:
    ```javascript
    store.service get {"store_name":"LineItem"} key={"l_orderkey":"100000","l_linenumber":"1"};
    ```
   








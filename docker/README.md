## Introduction
--------------------
This is the guide for the user to build docker images of RAF. For the user who is the new to docker, please refer this [guide](https://docs.docker.com/userguide/).

##Build RAF base images
----------
RAF base image is the enviroment that has all the dependcy libraries been built in. It is basic for the compile and development 
enviroment.

To build the RAF base images, please run `./build_raf_base_images`

After the base images is built, you can use `docker images` command to see the `raf_base` image. Now you can run a container 
from `raf_base` images. In this container you can use any source version controll software to download the RAF code or you can
use the following step to download and build RAF

 * Note: You must remove the proxy in the `docker_base/Dockerfile` if you don't use the proxy server or you must change the 
   proxy server name in the docker_base/Dockerfile if you use another proxy. 

##Buld RAF compile images
----------
After the base image is built, you can run ./build_raf_compiled_images to build the RAF compile images. This images will download
the source code from [RAF Git](https://github.com/RealtimeAnalyticsFramework/raf.git) and build the RAF source code. After the image 
has been built, you can use `docker images` command to see the `raf_compiled` images is in your image repository.
 
 * Note: the `docker_compile/settings.xml` is the configuration for the maven, which will be copied into the image, you must
   change the proxy name in the settings.xml file or you must remove the proxy if you don't use proxy server. Also 
   `docker_compile/Dockerfile` contains proxy setting, you must do the same action for this file.

FROM ubuntu

ENV HTTP_PROXY hd-cloud.intel.com:911
ENV HTTPS_PROXY hd-cloud.intel.com:911
RUN export http_proxy=$HTTP_PROXY && export https_proxy=$HTTPS_PROXY && \
    touch /etc/apt/apt.conf && \
    echo "Acquire::http::proxy \"http://hd-cloud.intel.com:911\";" > /etc/apt/apt.conf && \
    apt-get update -o Acquire::ForceIPv4=true && apt-get install -y \
        gcc \
        g++ \
        git \
        vim \
        make  \
        CMake  \
        unzip  \
        autotools-dev  \
        automake  \
        curl  \
        libtool  \
        pkg-config  \
        libnss3-dev  \
        libnss3-tools  \
        wget \
        maven \
        sshpass \
        libprotoc-dev  \
        libprotobuf-java  \
        protobuf-compiler \
        groff


RUN ssh-keygen -t rsa -N "" -C "" -f ~/.ssh/id_rsa
RUN ls ~/.ssh/
RUN sshpass  -p "123456"  scp -o StrictHostKeyChecking=no ~/.ssh/id_rsa.pub root@10.238.145.88:~/.ssh/authorized_keys

RUN mkdir /home/project
WORKDIR /home/Downloads

#RUN alias curl="curl --proxy hd-cloud.intel.com:911"
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'yajl-2.1.0.tar.gz' http://github.com/lloyd/yajl/tarball/2.1.0
 #install lloyd-yajl-2.1.0-0-ga0ecdde.tar.gz
 WORKDIR /home/Downloads/
 RUN tar -xvf yajl-2.1.0.tar.gz
 WORKDIR /home/Downloads/lloyd-yajl-66cb08c/
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf lloyd-yajl-66cb08c
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'asio-1.10.1.tar.gz' http://sourceforge.net/projects/asio/files/asio/1.10.1%20%28Stable%29/asio-1.10.1.tar.gz/download
 #install asio
 WORKDIR /home/Downloads/
 RUN tar -xvf asio-1.10.1.tar.gz
 WORKDIR asio-1.10.1
 RUN ./configure --without-boost && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf asio-1.10.1

RUN curl --proxy hd-cloud.intel.com:911 -L -o 'libqb-0.17.1.tar.gz' https://fedorahosted.org/releases/q/u/quarterback/libqb-0.17.1.tar.gz  
 #install libqb
 WORKDIR /home/Downloads/
 RUN tar -xvf libqb-0.17.1.tar.gz
 WORKDIR libqb-0.17.1
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf libqb-0.17.1
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'corosync-2.3.4.tar.gz' http://build.clusterlabs.org/corosync/releases/corosync-2.3.4.tar.gz
 #install corosycn
 WORKDIR /home/Downloads/
 RUN tar -xvf corosync-2.3.4.tar.gz && mv corosync-2.3.4 /home/project/
 WORKDIR  /home/project/corosync-2.3.4 
 RUN ./configure && make && make install 
 WORKDIR /home/Downloads/

RUN curl --proxy hd-cloud.intel.com:911 -L -o 'glog-0.3.3.tar.gz' https://google-glog.googlecode.com/files/glog-0.3.3.tar.gz 
 #install glog
 WORKDIR /home/Downloads/
 RUN tar -xvf glog-0.3.3.tar.gz
 WORKDIR glog-0.3.3
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf glog-0.3.3  
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'libunwind-1.1.tar.gz' http://download.savannah.gnu.org/releases/libunwind/libunwind-1.1.tar.gz
 #install libunwind
 WORKDIR /home/Downloads/
 RUN tar -xvf libunwind-1.1.tar.gz
 WORKDIR libunwind-1.1
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf libunwind-1.1 
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'gperftools-2.4.tar.gz' https://googledrive.com/host/0B6NtGsLhIcf7MWxMMF9JdTN3UVk/gperftools-2.4.tar.gz 
 #install gperftools 
 WORKDIR /home/Downloads/
 RUN tar -xvf gperftools-2.4.tar.gz
 WORKDIR gperftools-2.4
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf gperftools-2.4
 
#RUN curl --proxy hd-cloud.intel.com:911 -L -o 'protobuf-2.6.1.tar.gz' https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
 #install gperftools 
 #WORKDIR /home/Downloads/
 #RUN tar -xvf protobuf-2.6.1.tar.gz
 #WORKDIR protobuf-2.6.1
 #RUN ./configure && make && make install
 #WORKDIR /home/Downloads/
 #RUN rm -rf protobuf-2.6.1

RUN curl  --proxy hd-cloud.intel.com:911 -L -o 'protobuf-2.5.0.tar.gz' https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.gz
 #install gperftools
 WORKDIR /home/Downloads/
 RUN tar -xvf protobuf-2.5.0.tar.gz
 WORKDIR protobuf-2.5.0
 RUN ./configure && make && make install
 WORKDIR /home/Downloads/
 RUN rm -rf protobuf-2.5.0
 
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'gtest-1.7.0.zip' https://googletest.googlecode.com/files/gtest-1.7.0.zip 
 #setup gtest 
 WORKDIR /home/Downloads/
 RUN unzip gtest-1.7.0.zip && mv gtest-1.7.0 /home/project/
 WORKDIR /home/project
 WORKDIR gtest-1.7.0
 RUN ./configure && make
 WORKDIR lib
 RUN sed -i "s/\/home\/gtest-1.7.0\/libtool/\/usr\/bin\/libtool/g" libgtest_main.la
 WORKDIR .libs
 RUN cp libg* /usr/local/lib
 WORKDIR /home/Downloads/
 
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'tbb42_20140122oss_src.tgz' https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb42_20140122oss_src.tgz 
 #set tbb
 WORKDIR /home/Downloads/
 RUN tar -xvf tbb42_20140122oss_src.tgz && mv tbb42_20140122oss /home/project/
 WORKDIR /home/project/tbb42_20140122oss
 RUN make
 WORKDIR build/linux_intel64_gcc_cc4.8_libc2.19_kernel3.13.0_release/
 RUN cp lib* /usr/local/lib 
 WORKDIR /home/Downloads/

RUN curl --proxy hd-cloud.intel.com:911 -L -o 'udt.sdk.4.11.tar.gz' http://pkgs.fedoraproject.org/repo/pkgs/udt/udt.sdk.4.11.tar.gz/30b1556e5cf0afe179e40a53a1371b08/udt.sdk.4.11.tar.gz
 #install udt 
 WORKDIR /home/Downloads/
 RUN tar -xvf udt.sdk.4.11.tar.gz && mv udt4 /home/project/
 WORKDIR /home/project/udt4
 RUN make && make install
 WORKDIR src
 RUN cp lib* /usr/local/lib 
 WORKDIR /home/Downloads/ 

#setup gtest env and tbb41_20121003oss_lin.tgz
ENV GTEST_HOME /home/project/gtest-1.7.0
ENV TBB_HOME /home/project/tbb42_20140122oss
ENV UDT_HOME /home/project/udt4
ENV LD_LIBRARY_PATH /usr/local/lib:/usr/lib:$TBB_HOME/build/linux_intel64_gcc_cc4.8_libc2.19_kernel3.13.0_release

RUN export GTEST_HOME && export TBB_HOME &&export UDT_HOME && export LD_LIBRARY_PATH && \
    curl --proxy hd-cloud.intel.com:911 -b oraclelicense=accept-securebackup-cookie -L -o 'jdk-8u31-linux-x64.tar.gz' http://download.oracle.com/otn-pub/java/jdk/8u31-b13/jdk-8u31-linux-x64.tar.gz && \
    tar -xvf jdk-8u31-linux-x64.tar.gz && \
    mv jdk1.8.0_31 /home/project/

ENV JAVE_HOME /home/project/jdk1.8.0_31
ENV PATH $PATH:$JAVE_HOME/bin
RUN rm -rf *
WORKDIR /home 

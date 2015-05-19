FROM ubuntu

COPY install_libs /opt/

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

WORKDIR /opt/
RUN ./install_libs		
		
#setup gtest env and tbb41_20121003oss_lin.tgz
ENV GTEST_HOME /home/project/gtest-1.7.0
ENV TBB_HOME /home/project/tbb42_20140122oss
ENV UDT_HOME /home/project/udt4
ENV LD_LIBRARY_PATH /usr/local/lib:/usr/lib:$TBB_HOME/build/linux_intel64_gcc_cc4.8_libc2.19_kernel3.13.0_release

RUN export GTEST_HOME && export TBB_HOME &&export UDT_HOME && export LD_LIBRARY_PATH

ENV JAVE_HOME /home/project/jdk1.8.0_31
ENV PATH $PATH:$JAVE_HOME/bin


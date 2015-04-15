FROM raf_base

#ENV HTTP_PROXY proxy-us.intel.com:912
#ENV HTTPS_PROXY proxy-us.intel.com:912

#RUN export http_proxy=$HTTP_PROXY && export https_proxy=$HTTPS_PROXY && \
#    touch /etc/apt/apt.conf && \
#    echo "Acquire::http::proxy \"htpp://proxy-us.intel.com:912\";" > /etc/apt/apt.conf && \
#    apt-get update -o Acquire::ForceIPv4=true && \
#    apt-get install -y eclipse

COPY settings.xml /etc/maven/settings.xml
WORKDIR /home/project

RUN unset HTTP_PROXY
RUN unset HTTPS_PROXY
RUN git clone root@10.10.10.201:/var/git/idgs.git
WORKDIR /home/project/idgs
RUN ./build/build.sh
RUN sleep 2
RUN ./build/it.sh

ENV HTTP_PROXY proxy-us.intel.com:912
ENV HTTPS_PROXY proxy-us.intel.com:912

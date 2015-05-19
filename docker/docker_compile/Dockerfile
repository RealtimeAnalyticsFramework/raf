FROM raf_base

COPY settings.xml /etc/maven/settings.xml
COPY corosync.conf /etc/corosync/corosync.conf
COPY startIT /home/project/
COPY limits.conf /etc/security/limits.conf
WORKDIR /home/project

ENV HTTP_PROXY hd-cloud.intel.com:911
ENV HTTPS_PROXY hd-cloud.intel.com:911


RUN curl --proxy hd-cloud.intel.com:911 -L -o 'yajl-2.1.0.tar.gz' http://github.com/lloyd/yajl/tarball/2.1.0
 #install lloyd-yajl-2.1.0-0-ga0ecdde.tar.gz
 WORKDIR /home/project/
 RUN tar -xvf yajl-2.1.0.tar.gz
 WORKDIR /home/project/lloyd-yajl-66cb08c/
 RUN ./configure && make && make install
 WORKDIR /home/project/
 RUN rm -rf lloyd-yajl-66cb08c

RUN git clone https://github.com/RealtimeAnalyticsFramework/raf.git
RUN mkdir idgs
RUN cp -rf raf/raf/* idgs/
ENV IDGS_HOME /home/project/idgs/dist
WORKDIR /home/project/idgs
RUN mkdir -p /home/project/idgs/dist/conf
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'jdo2-api-2.3-ec.jar' http://www.datanucleus.org/downloads/maven2/javax/jdo/jdo2-api/2.3-ec/jdo2-api-2.3-ec.jar
RUN mkdir -p /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN mv jdo2-api-2.3-ec.jar /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN ./build/build.sh


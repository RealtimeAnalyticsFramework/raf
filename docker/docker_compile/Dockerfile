FROM raf_base

COPY settings.xml /etc/maven/settings.xml
WORKDIR /home/project

RUN unset HTTP_PROXY
RUN unset HTTPS_PROXY
RUN git clone root@10.10.10.201:/var/git/idgs.git
ENV HTTP_PROXY hd-cloud.intel.com:911
ENV HTTPS_PROXY hd-cloud.intel.com:911
ENV IDGS_HOME /home/project/idgs/dist
WORKDIR /home/project/idgs/
RUN mkdir -p /home/project/idgs/dist/conf
#RUN cp /home/project/idgs/samples/load/conf/client.conf /home/project/idgs/dist/conf/
RUN curl --proxy hd-cloud.intel.com:911 -L -o 'jdo2-api-2.3-ec.jar' http://www.datanucleus.org/downloads/maven2/javax/jdo/jdo2-api/2.3-ec/jdo2-api-2.3-ec.jar
RUN mkdir -p /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN mv jdo2-api-2.3-ec.jar /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN ./build/build.sh


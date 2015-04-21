FROM raf_compiled

WORKDIR /home/project
COPY corosync.conf /etc/corosync/corosync.conf
RUN curl --proxy proxy:911 -L -o 'jdo2-api-2.3-ec.jar' http://www.datanucleus.org/downloads/maven2/javax/jdo/jdo2-api/2.3-ec/jdo2-api-2.3-ec.jar
RUN mkdir -p /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN mv jdo2-api-2.3-ec.jar /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
WORKDIR /home/project/idgs
RUN ./build/it.sh


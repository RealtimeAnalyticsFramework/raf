FROM raf_base

COPY settings.xml /etc/maven/settings.xml
WORKDIR /home/project

ENV HTTP_PROXY proxy:911
ENV HTTPS_PROXY proxy:911
RUN git clone https://github.com/RealtimeAnalyticsFramework/raf.git
ENV IDGS_HOME /home/project/raf/raf/dist
WORKDIR /home/project/raf/raf
RUN mkdir -p /home/project/raf/raf/dist/conf
RUN curl --proxy proxy:911 -L -o 'jdo2-api-2.3-ec.jar' http://www.datanucleus.org/downloads/maven2/javax/jdo/jdo2-api/2.3-ec/jdo2-api-2.3-ec.jar
RUN mkdir -p /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN mv jdo2-api-2.3-ec.jar /root/.m2/repository/javax/jdo/jdo2-api/2.3-ec/
RUN ./build/build.sh


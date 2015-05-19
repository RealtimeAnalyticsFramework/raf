
FROM raf_compiled

ENV HTTP_PROXY hd-cloud.intel.com:911
ENV HTTPS_PROXY hd-cloud.intel.com:911
RUN export http_proxy=$HTTP_PROXY && export https_proxy=$HTTPS_PROXY && \
    touch /etc/apt/apt.conf && \
    echo "Acquire::http::proxy \"http://hd-cloud.intel.com:911\";" > /etc/apt/apt.conf && \
    #apt-get update -o Acquire::ForceIPv4=true && \
    apt-get install -y eclipse && apt-get install -qqy x11-apps
ENV DISPLAY :0
CMD /usr/bin/X11/./eclipse

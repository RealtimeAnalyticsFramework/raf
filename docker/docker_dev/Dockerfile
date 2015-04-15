
FROM raf_base

ENV HTTP_PROXY proxy-us.intel.com:912
ENV HTTPS_PROXY proxy-us.intel.com:912
RUN export http_proxy=$HTTP_PROXY && export https_proxy=$HTTPS_PROXY && \
    touch /etc/apt/apt.conf && \
    echo "Acquire::http::proxy \"htpp://proxy-us.intel.com:912\";" > /etc/apt/apt.conf && \
    apt-get update -o Acquire::ForceIPv4=true && \
    apt-get install -y eclipse

RUN apt-get install -qqy x11-apps
ENV DISPLAY :0
CMD eclipse

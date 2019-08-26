FROM alpine:latest
RUN apk update && \
  apk add \
    git \
    vim \
    alpine-sdk \
    bash \
    libffi libffi-dev \
    openssl openssl-dev \
    python2 python2-dev py2-pip
RUN mkdir ~/esp && cd ~/esp && git clone https://github.com/espressif/esp-idf.git
COPY bashrc ~/.bashrc
RUN /bin/bash ~/esp/esp-idf/install.sh


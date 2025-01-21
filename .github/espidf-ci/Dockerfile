FROM ubuntu:latest

ENV ESPIDF_PATH=/opt/esp-idf
ENV ESP8266_SDK_PATH=/opt/ESP8266_RTOS_SDK

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y \
    cmake \
    git \
    libusb-1.0-0 \
    make \
    ninja-build \
    nodejs \
    python3 \
    python3-pip \
    python3-venv \
    python3-virtualenv \
    sudo
RUN apt-get clean

RUN cd /usr/bin && \
  ln -sf python3 python && \
  ln -sf pip3 pip

RUN ln -sf /bin/bash /bin/sh

# install ESP IDF (root user)
RUN git clone -- https://github.com/espressif/esp-idf ${ESPIDF_PATH} && \
  cd ${ESPIDF_PATH} && \
  git submodule update --init --recursive

# install ESP8266 RTOS SDK (root user)
RUN git clone -- https://github.com/espressif/ESP8266_RTOS_SDK ${ESP8266_SDK_PATH} && \
  cd ${ESP8266_SDK_PATH} && \
  git submodule update --init --recursive

FROM ubuntu:latest

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y \
    cmake \
    git \
    libusb-1.0-0 \
    make \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    sudo
RUN apt-get clean

RUN pip3 install pyserial

RUN cd /usr/bin && \
  ln -sf python3 python && \
  ln -sf pip3 pip

RUN ln -sf /bin/bash /bin/sh

RUN cd / && \
  git clone https://github.com/espressif/esp-idf && \
  cd esp-idf && \
  git checkout v5.1.1 && \
  git submodule update --init --recursive

RUN cd /esp-idf && \
  ./install.sh

RUN cd / && \
  git clone https://github.com/espressif/ESP8266_RTOS_SDK && \
  cd ESP8266_RTOS_SDK && \
  git checkout v3.4 && \
  git submodule update --init --recursive

RUN cd /ESP8266_RTOS_SDK && \
  ./install.sh

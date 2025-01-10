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

RUN git clone -- https://github.com/espressif/esp-idf /esp-idf && \
  cd /esp-idf && \
  git submodule update --init --recursive && \
  chmod --recursive a+rw .

RUN git clone -- https://github.com/espressif/ESP8266_RTOS_SDK /ESP8266_RTOS_SDK && \
  cd /ESP8266_RTOS_SDK && \
  git submodule update --init --recursive && \
  chmod --recursive a+rw .

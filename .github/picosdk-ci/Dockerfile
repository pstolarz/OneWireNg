FROM ubuntu:latest

ENV PICO_SDK_PATH=/opt/pico-sdk

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y \
    cmake \
    g++ \
    gcc-arm-none-eabi \
    git \
    make \
    nodejs \
    python3 \
    sudo
RUN apt-get clean

RUN cd /usr/bin && \
  ln -sf python3 python && \
  ln -sf pip3 pip

RUN ln -sf /bin/bash /bin/sh

# install Pico SDK (root user)
RUN git clone -- https://github.com/raspberrypi/pico-sdk ${PICO_SDK_PATH} && \
  cd ${PICO_SDK_PATH} && \
  git submodule update --init --recursive

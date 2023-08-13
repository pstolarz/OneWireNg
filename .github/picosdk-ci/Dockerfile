FROM ubuntu:latest

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y \
    git

ENV PICO_SDK_PATH=/pico-sdk

RUN cd / && \
  git clone https://github.com/raspberrypi/pico-sdk && \
  cd pico-sdk && \
  git checkout 1.5.1 && \
  git submodule update --init --recursive

RUN apt-get update && \
  apt-get install -y \
    cmake \
    g++ \
    gcc-arm-none-eabi \
    make \
    python3 \
    sudo
RUN apt-get clean

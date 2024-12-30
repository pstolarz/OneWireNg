FROM ubuntu:latest

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

RUN git clone -- https://github.com/raspberrypi/pico-sdk /pico-sdk && \
  cd /pico-sdk && \
  git submodule update --init --recursive && \
  chmod --recursive a+rw .

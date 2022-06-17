FROM ubuntu:latest

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y \
    curl \
    python3 \
    python3-pip \
    sudo

RUN cd /usr/bin && \
  ln -sf python3 python && \
  ln -sf pip3 pip

RUN ln -sf /bin/bash /bin/sh

RUN python -c \
  "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"

RUN ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio && \
  ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio && \
  ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb

RUN mkdir /pio-ard-ci && cd /pio-ard-ci && \
  pio project init -s && \
  echo                                  >>platformio.ini && \
  echo "[env]"                          >>platformio.ini && \
  echo "framework = arduino"            >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:uno]"                      >>platformio.ini && \
  echo "platform = atmelavr"            >>platformio.ini && \
  echo "board = uno"                    >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:uno_wifi_rev2]"            >>platformio.ini && \
  echo "platform = atmelmegaavr"        >>platformio.ini && \
  echo "board = uno_wifi_rev2"          >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:due]"                      >>platformio.ini && \
  echo "platform = atmelsam"            >>platformio.ini && \
  echo "board = due"                    >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:esp32dev]"                 >>platformio.ini && \
  echo "platform = espressif32"         >>platformio.ini && \
  echo "board = esp32dev"               >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:d1]"                       >>platformio.ini && \
  echo "platform = espressif8266"       >>platformio.ini && \
  echo "board = d1"                     >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:nucleo_l552ze_q]"          >>platformio.ini && \
  echo "platform = ststm32"             >>platformio.ini && \
  echo "board = nucleo_l552ze_q"        >>platformio.ini && \
  pio pkg install

RUN mkdir /pio-idf-ci && cd /pio-idf-ci && \
  pio project init -s && \
  echo                                  >>platformio.ini && \
  echo "[env:esp32-s2-saola-1]"         >>platformio.ini && \
  echo "framework = espidf"             >>platformio.ini && \
  echo "platform = espressif32"         >>platformio.ini && \
  echo "board = esp32-s2-saola-1"       >>platformio.ini && \
  echo                                  >>platformio.ini && \
  echo "[env:esp32-c3-devkitm-1]"       >>platformio.ini && \
  echo "framework = espidf"             >>platformio.ini && \
  echo "platform = espressif32"         >>platformio.ini && \
  echo "board = esp32-c3-devkitm-1"     >>platformio.ini && \
  pio pkg install

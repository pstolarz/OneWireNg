FROM ubuntu:latest

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
  apt-get install -y curl

# install arduino-cli
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

RUN arduino-cli config init

# install standard cores
RUN arduino-cli core install arduino:avr
RUN arduino-cli core install arduino:megaavr
RUN arduino-cli core install arduino:sam
RUN arduino-cli core install arduino:samd
RUN arduino-cli core install arduino:mbed_edge
RUN arduino-cli core install arduino:mbed_nano
RUN arduino-cli core install arduino:mbed_nicla
RUN arduino-cli core install arduino:mbed_portenta
RUN arduino-cli core install arduino:mbed_rp2040

# install extra cores
RUN arduino-cli config add board_manager.additional_urls \
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json && \
  arduino-cli config add board_manager.additional_urls \
    http://arduino.esp8266.com/stable/package_esp8266com_index.json && \
  arduino-cli config add board_manager.additional_urls \
    https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/main/package_stmicroelectronics_index.json && \
  arduino-cli config add board_manager.additional_urls \
    http://drazzy.com/package_drazzy.com_index.json && \
  arduino-cli update

RUN arduino-cli core install esp32:esp32
RUN arduino-cli core install esp8266:esp8266
RUN arduino-cli core install STMicroelectronics:stm32
RUN arduino-cli core install megaTinyCore:megaavr

RUN apt-get install -y \
  python3 \
  python3-pip \
  sudo \
  zip
RUN apt-get clean

# ESP-IDF required
RUN pip3 install pyserial
RUN cd /usr/bin && \
  ln -sf python3 python && \
  ln -sf pip3 pip

RUN ln -sf /bin/bash /bin/sh

RUN arduino-cli config set library.enable_unsafe_install true

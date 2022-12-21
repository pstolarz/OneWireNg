# OneWireNg examples

### `DallasTemperature`
[*Arduino*](arduino/DallasTemperature/DallasTemperature.ino) |
[*ESP-IDF*](esp-idf/DallasTemperature/main/DallasTemperature.cpp) |
[*Pico SDK*](pico-sdk/DallasTemperature/DallasTemperature.cpp) |
[*Mbed OS*](mbed-os/DallasTemperature/DallasTemperature.cpp)

Dallas temperature sensors example. Scan the bus for all supported types of
slaves. Convert temperature for all sensors simultaneously and read them
one-by-one. Possible permanent configuration change. The example uses generic
Dallas thermometers driver [`DSTherm`](../src/drivers/DSTherm.h).

---
### `MAX31850`
[*Arduino*](arduino/MAX31850/MAX31850.ino)

MAX31850/MAX31851 thermocouple example. Scan the bus for supported
slaves. Convert temperature for all sensors simultaneously and read them
one-by-one. The example uses [`MAX31850`](../src/drivers/MAX31850.h) driver.

---
### `OverdriveMode`

[*Arduino*](arduino/OverdriveMode/OverdriveMode.ino)

Basic overdrive example. Scan the bus for overdrive enabled devices.

---
### `DS2431`
[*Arduino*](arduino/DS2431/DS2431.ino) |
[*ESP-IDF*](esp-idf/DS2431/main/DS2431.cpp) |
[*Pico SDK*](pico-sdk/DS2431/DS2431.cpp) |
[*Mbed OS*](mbed-os/DS2431/DS2431.cpp)

Simple DS2431 device (EEPROM memory) example. Read + write. Overdrive mode
support.

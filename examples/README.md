# OneWireNg examples

### [`DallasTemperature`](DallasTemperature/DallasTemperature.ino)

Dallas temperature sensors example. Scan the bus for all supported types of
slaves. Convert temperature for all sensors simultaneously and read them
one-by-one. Possible permanent configuration change. Example uses generic
Dallas thermometers driver [`DSTherm`](../src/drivers/DSTherm.h).

### [`OverdriveMode`](OverdriveMode/OverdriveMode.ino)

Basic overdrive example. Scan the bus for overdrive enabled devices.

### [`DS2431`](DS2431/DS2431.ino)

Simple DS2431 device (EEPROM memory) example. Read + write. Overdrive mode
support.

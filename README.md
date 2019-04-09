# OneWireNg

Ths is an Arduino 1-wire service library, intended as an alternative for the
classic [OneWire](https://github.com/PaulStoffregen/OneWire) library. The library
provides basic 1-wire services (reset, search, touch, read, write, parasite
powering) and may serve for further work while interfacing with various 1-wire
devices (e.g. Dallas/Maxim thermometers).

## Features

* All bus activities are performed respecting open-drain character of the 1-wire
  protocol.

  During normal 1-wire activities, the master MCU GPIO controlling the bus is
  never set high (providing direct voltage source on the bus) instead the GPIO
  is switched to the reading mode causing the high state seen on the bus via
  the pull-up resistor.

* 1-wire touch support.

  The 1-wire touch may substantially simplify complex bus activities consisting
  of write-read pairs by combining them into a single touch activity. See examples
  for details.

* Parasite powering support.

  The 1-wire bus may be powered directly by the master MCU GPIO or via a switching
  transistor controlled by a dedicated MCU GPIO, if the target platform's GPIOs
  (in the output mode) work in the open-drain mode only. More details
  [below](#parasite-powering).

* Clear and flexible architecture.

  The code architecture allows fast and easy porting for new Arduino platforms
  or even usage of the core part of library outside the Arduino environment.

## Supported platforms

Currently Arduino AVR only, since it's the only platform I'm able to test for.
Expect more platforms support in the future. **I'm inviting all developers**,
eager to help me with porting and testing the library for new platforms.

## Architecture details

### OneWireNg

The class provides public interface for 1-wire service. Object of this class
isn't constructed directly rather than casted from a derived class object
implementing platform specific details.

As an example:

```cpp
OneWireNg::Id id;
OneWireNg::ErrorCode ec;

OneWireNg *ow = new OneWireNg_ArduinoAVR(10);
do
{
    ec = ow->search(id);
    if (ec == OneWireNg::EC_MORE || ec == OneWireNg::EC_DONE) {
        // `id' contains 1-wire address of a connected slave
    }
} while (ec == OneWireNg::EC_MORE);
```

creates 1-wire service interface for Arduino AVR platform and perform search on
the bus. The bus is controlled by Arduino pin number 10.

### OneWireNg_BitBang

The class is derived from `OneWireNg` and implements the 1-wire interface basing
on GPIO bit-banging. Object of this class isn't constructed directly rather than
the class is intended to be inherited by a derived class providing protected
interface implementation for low level GPIO activities (set mode, read, write).

### OneWireNg_PLATFORM

Are family of classes providing platform specific implementation (`PLATFORM`
states for a platform name e.g. `OneWireNg_ArduinoAVR` provides AVR implementation
for Arduino environment).

The platform classes implement `OneWireNg` interface directly (via direct
`OneWireNg` class inheritance) or indirectly (e.g. GPIO bit-banging implementation
bases on `OneWireNg_BitBang`, which provides GPIO bit-banging 1-wire service
implementation leaving the platform class to provide platform specific low-level
GPIO activities details).

Platform classes have a public constructor allowing to create 1-wire service for
a particular platform (see [above](#architecture-details)).

## Usage

Refer to `examples` directory for usage details.
For API details refer to sources inline docs (mainly `OneWireNg` class).

## Parasite powering

TODO

## License

2 clause BSD license. See LICENSE file for details.

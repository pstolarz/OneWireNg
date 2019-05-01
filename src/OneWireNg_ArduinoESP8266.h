/*
 * Copyright (c) 2019 Piotr Stolarz
 * OneWireNg: Ardiono 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __OWNG_ARDUINO_ESP8266__
#define __OWNG_ARDUINO_ESP8266__

#include "Arduino.h"
#include "OneWireNg_BitBang.h"

#define __WRITE_GPIO(pin, st) \
    if (pin < 16) { \
        if (st) GPOS = (1 << pin); else GPOC = (1 << pin); \
    } else \
    if (pin == 16) { \
        if (st) GP16O |= 1; else GP16O &= ~1; \
    }

#define __GPIO_AS_INPUT(pin) \
    if (pin < 16) { \
        GPEC = (1 << pin); \
    } else \
    if (pin == 16) { \
      GP16E &= ~1; \
    }

#define __GPIO_AS_OUTPUT(pin) \
    if (pin < 16) { \
        GPES = (1 << pin); \
    } else \
    if (pin == 16) { \
      GP16E |= 1; \
    }

/**
 * Arduino ESP8266 platform GPIO specific implementation.
 */
class OneWireNg_ArduinoESP8266: public OneWireNg_BitBang
{
public:
    /**
     * OneWireNg 1-wire service for Arduino ESP8266 platform.
     *
     * Bus powering is supported via switching its GPIO to the high state.
     * In this case the GPIO servers as a voltage source for connected slaves
     * working in parasite powering configuration.
     *
     * @param pin Arduino GPIO pin number used for bit-banging 1-wire bus.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_ArduinoESP8266(unsigned pin, bool pullUp):
        OneWireNg_BitBang(false)
    {
        _dtaPin = pin;
        initDtaGpio(pullUp);
    }

    /**
     * OneWireNg 1-wire service for Arduino ESP8266 platform.
     *
     * Bus powering is supported via a switching transistor providing
     * the power to the bus and controlled by a dedicated GPIO (see @sa
     * OneWireNg_BitBang::setupPwrCtrlGpio()). In this configuration the
     * service mimics the open-drain type of output. The approach may be
     * feasible if the GPIO is unable to provide sufficient power for
     * connected slaves working in parasite powering configuration.
     *
     * @param pin Arduino GPIO pin number used for bit-banging 1-wire bus.
     * @param pwrCtrlPin Arduino GPIO pin number controlling the switching
     *     transistor.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_ArduinoESP8266(unsigned pin, unsigned pwrCtrlPin, bool pullUp):
        OneWireNg_BitBang(true)
    {
        _dtaPin = pin;
        _pwrCtrlPin = pwrCtrlPin;

        initDtaGpio(pullUp);
        initPwrCtrlGpio();
    }

protected:
    virtual int readGpioIn(GpioType gpio)
    {
        UNUSED(gpio);
        return (_dtaPin < 16 ? GPIP(_dtaPin) :
            _dtaPin == 16 ? (GP16I & 0x01) : 1);
    }

    virtual void writeGpioOut(GpioType gpio, int state)
    {
        if (gpio == GPIO_DTA) {
            __WRITE_GPIO(_dtaPin, state);
        } else {
            __WRITE_GPIO(_pwrCtrlPin, state);
        }
    }

    virtual void setGpioAsInput(GpioType gpio)
    {
        UNUSED(gpio);
        __GPIO_AS_INPUT(_dtaPin);
    }

    virtual void setGpioAsOutput(GpioType gpio, int state)
    {
        if (gpio == GPIO_DTA) {
            __GPIO_AS_OUTPUT(_dtaPin);
            __WRITE_GPIO(_dtaPin, state);
        } else {
            __GPIO_AS_OUTPUT(_pwrCtrlPin);
            __WRITE_GPIO(_pwrCtrlPin, state);
        }
    }

    void initDtaGpio(bool pullUp)
    {
        pinMode(_dtaPin, (pullUp ? INPUT_PULLUP : INPUT));
        setupDtaGpio();
    }

    void initPwrCtrlGpio(void)
    {
        pinMode(_pwrCtrlPin, OUTPUT_OPEN_DRAIN);
        setupPwrCtrlGpio(true);
    }

private:
    unsigned _dtaPin;
    unsigned _pwrCtrlPin;
};

#undef __GPIO_AS_OUTPUT
#undef __GPIO_AS_INPUT
#undef __WRITE_GPIO

#endif /* __OWNG_ARDUINO_ESP8266__ */

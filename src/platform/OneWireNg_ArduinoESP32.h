/*
 * Copyright (c) 2019,2020 Piotr Stolarz
 * OneWireNg: Ardiono 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __OWNG_ARDUINO_ESP32__
#define __OWNG_ARDUINO_ESP32__

#include <assert.h>
#include "Arduino.h"
#include "OneWireNg_BitBang.h"

#if (F_CPU < 160000000L)
# warning "1-wire dysfunctional for CPU freq. less than 160MHz"
#endif
#if defined(CONFIG_OVERDRIVE_ENABLED) && (F_CPU < 240000000)
# warning "240MHz CPU freq. is recommended for overdrive mode"
#endif

#define __READ_GPIO(gs) \
    ((*gs.inReg & gs.bmsk) != 0)

#define __WRITE0_GPIO(gs) \
    *gs.outClrReg = gs.bmsk

#define __WRITE1_GPIO(gs) \
    *gs.outSetReg = gs.bmsk

#define __WRITE_GPIO(gs, st) \
    if (st) __WRITE1_GPIO(gs); \
    else __WRITE0_GPIO(gs)

#define __GPIO_AS_INPUT(gs) \
    *gs.modClrReg = gs.bmsk

#define __GPIO_AS_OUTPUT(gs) \
    *gs.modSetReg = gs.bmsk

/**
 * Arduino ESP32 platform GPIO specific implementation.
 */
class OneWireNg_ArduinoESP32: public OneWireNg_BitBang
{
public:
    /**
     * OneWireNg 1-wire service for Arduino ESP32 platform.
     *
     * Bus powering is supported via switching its GPIO to the high state.
     * In this case the GPIO servers as a voltage source for connected slaves
     * working in parasite powering configuration.
     *
     * @param pin Arduino GPIO pin number used for bit-banging 1-wire bus.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_ArduinoESP32(unsigned pin, bool pullUp):
        OneWireNg_BitBang(false)
    {
        initDtaGpio(pin, pullUp);
    }

    /**
     * OneWireNg 1-wire service for Arduino ESP32 platform.
     *
     * Bus powering is supported via a switching transistor providing
     * the power to the bus and controlled by a dedicated GPIO (@see
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
    OneWireNg_ArduinoESP32(unsigned pin, unsigned pwrCtrlPin, bool pullUp):
        OneWireNg_BitBang(true)
    {
        initDtaGpio(pin, pullUp);
        initPwrCtrlGpio(pwrCtrlPin);
    }

protected:
    virtual int readGpioIn(GpioType gpio)
    {
        UNUSED(gpio);
        return __READ_GPIO(_dtaGpio);
    }

    virtual void writeGpioOut(GpioType gpio, int state)
    {
        if (gpio == GPIO_DTA) {
            __WRITE_GPIO(_dtaGpio, state);
        } else {
            __WRITE_GPIO(_pwrCtrlGpio, state);
        }
    }

    virtual void setGpioAsInput(GpioType gpio)
    {
        UNUSED(gpio);
        __GPIO_AS_INPUT(_dtaGpio);
    }

    virtual void setGpioAsOutput(GpioType gpio, int state)
    {
        if (gpio == GPIO_DTA) {
            __WRITE_GPIO(_dtaGpio, state);
            __GPIO_AS_OUTPUT(_dtaGpio);
        } else {
            __WRITE_GPIO(_pwrCtrlGpio, state);
            __GPIO_AS_OUTPUT(_pwrCtrlGpio);
        }
    }

#ifdef CONFIG_OVERDRIVE_ENABLED
    virtual int touch1Overdrive()
    {
        __WRITE0_GPIO(_dtaGpio);
        __GPIO_AS_OUTPUT(_dtaGpio);
        /* 0.5-1.5 usec at nominal freq. */
# if (F_CPU >= 240000000L)
        delayMicroseconds(0);
# endif

        /* speed up low-to-high transition */
        __WRITE1_GPIO(_dtaGpio);
        __GPIO_AS_INPUT(_dtaGpio);
        /* start sampling at ~2 usec at nominal freq. */
        return __READ_GPIO(_dtaGpio);
    }
#endif

    void initDtaGpio(unsigned pin, bool pullUp)
    {
        /* pins above 33 can only be inputs */
        assert(pin < 34);

        if (pin < 32) {
            _dtaGpio.bmsk = (uint32_t)(1UL << pin);
            _dtaGpio.inReg = &GPIO.in;
            _dtaGpio.outSetReg = &GPIO.out_w1ts;
            _dtaGpio.outClrReg = &GPIO.out_w1tc;
            _dtaGpio.modSetReg = &GPIO.enable_w1ts;
            _dtaGpio.modClrReg = &GPIO.enable_w1tc;
        } else {
            _dtaGpio.bmsk = (uint32_t)(1UL << (pin-32));
            _dtaGpio.inReg = &GPIO.in1.val;
            _dtaGpio.outSetReg = &GPIO.out1_w1ts.val;
            _dtaGpio.outClrReg = &GPIO.out1_w1tc.val;
            _dtaGpio.modSetReg = &GPIO.enable1_w1ts.val;
            _dtaGpio.modClrReg = &GPIO.enable1_w1tc.val;
        }
        pinMode(pin, INPUT | (pullUp ? PULLUP : 0));
        setupDtaGpio();
    }

    void initPwrCtrlGpio(unsigned pin)
    {
        /* pins above 33 can only be inputs */
        assert(pin < 34);

        if (pin < 32) {
            _pwrCtrlGpio.bmsk = (uint32_t)(1UL << pin);
            _pwrCtrlGpio.outSetReg = &GPIO.out_w1ts;
            _pwrCtrlGpio.outClrReg = &GPIO.out_w1tc;
            _pwrCtrlGpio.modSetReg = &GPIO.enable_w1ts;
            _pwrCtrlGpio.modClrReg = &GPIO.enable_w1tc;
        } else {
            _pwrCtrlGpio.bmsk = (uint32_t)(1UL << (pin-32));
            _pwrCtrlGpio.outSetReg = &GPIO.out1_w1ts.val;
            _pwrCtrlGpio.outClrReg = &GPIO.out1_w1tc.val;
            _pwrCtrlGpio.modSetReg = &GPIO.enable1_w1ts.val;
            _pwrCtrlGpio.modClrReg = &GPIO.enable1_w1tc.val;
        }
        pinMode(pin, OUTPUT);
        setupPwrCtrlGpio(true);
    }

private:
    struct {
        uint32_t bmsk;
        volatile uint32_t *inReg;
        volatile uint32_t *outSetReg;
        volatile uint32_t *outClrReg;
        volatile uint32_t *modSetReg;
        volatile uint32_t *modClrReg;
    } _dtaGpio;

    struct {
        uint32_t bmsk;
        volatile uint32_t *outSetReg;
        volatile uint32_t *outClrReg;
        volatile uint32_t *modSetReg;
        volatile uint32_t *modClrReg;
    } _pwrCtrlGpio;
};

#undef __GPIO_AS_OUTPUT
#undef __GPIO_AS_INPUT
#undef __WRITE_GPIO
#undef __READ_GPIO

#endif /* __OWNG_ARDUINO_ESP32__ */

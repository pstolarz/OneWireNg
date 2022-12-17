/*
 * Copyright (c) 2022 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __OWNG_PICO_RP2040__
#define __OWNG_PICO_RP2040__

#include "OneWireNg_BitBang.h"
#include "platform/Platform_TimeCritical.h"

#include "hardware/gpio.h"

#ifdef __MBED__
# define __GPIO_INIT(gpio) _gpio_init(gpio)
#else
# define __GPIO_INIT(gpio) gpio_init(gpio)
#endif

#define __READ_GPIO(bmsk) \
    ((sio_hw->gpio_in & bmsk) != 0)

#define __WRITE0_GPIO(bmsk) \
    sio_hw->gpio_clr = bmsk

#define __WRITE1_GPIO(bmsk) \
    sio_hw->gpio_set = bmsk

#define __WRITE_GPIO(bmsk, st) \
    if (st) __WRITE1_GPIO(bmsk); \
    else __WRITE0_GPIO(bmsk)

/**
 * RP2040's GPIO specific implementation.
 *
 * @note The driver uses Pico SDK API. Since the SDK is part of Arduino
 *     framework, the driver may be used for both of these frameworks.
 */
class OneWireNg_PicoRP2040: public OneWireNg_BitBang
{
public:
    /**
     * OneWireNg 1-wire service for RP2040 platform (GPIO bit-banging).
     *
     * Bus powering is supported via switching its GPIO to the high state.
     * In this case the GPIO servers as a voltage source for connected slaves
     * working in parasite powering configuration.
     *
     * @param pin GPIO pin number used for bit-banging 1-wire bus.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_PicoRP2040(unsigned pin, bool pullUp)
    {
        initDtaGpio(pin, pullUp);
    }

#if CONFIG_PWR_CTRL_ENABLED
    /**
     * OneWireNg 1-wire service for RP2040 platform (GPIO bit-banging).
     *
     * Bus powering is supported via a switching transistor providing
     * the power to the bus and controlled by a dedicated GPIO (@see
     * OneWireNg_BitBang::setupPwrCtrlGpio()). In this configuration the
     * service mimics the open-drain type of output. The approach may be
     * feasible if the GPIO is unable to provide sufficient power for
     * connected slaves working in parasite powering configuration.
     *
     * @param pin GPIO pin number used for bit-banging 1-wire bus.
     * @param pwrCtrlPin GPIO pin number controlling the switching transistor.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_PicoRP2040(unsigned pin, unsigned pwrCtrlPin, bool pullUp)
    {
        initDtaGpio(pin, pullUp);
        initPwrCtrlGpio(pwrCtrlPin);
    }
#endif

protected:
    TIME_CRITICAL int readDtaGpioIn()
    {
        return __READ_GPIO(_dtaGpio_bmsk);
    }

    TIME_CRITICAL void setDtaGpioAsInput()
    {
        gpio_set_dir_in_masked(_dtaGpio_bmsk);
    }

#if CONFIG_PWR_CTRL_ENABLED
    TIME_CRITICAL void writeGpioOut(int state, GpioType gpio)
    {
        if (gpio == GPIO_DTA) {
            __WRITE_GPIO(_dtaGpio_bmsk, state);
        } else {
            __WRITE_GPIO(_pwrCtrlGpio_bmsk, state);
        }
    }

    TIME_CRITICAL void setGpioAsOutput(int state, GpioType gpio)
    {
        if (gpio == GPIO_DTA) {
            __WRITE_GPIO(_dtaGpio_bmsk, state);
            gpio_set_dir_out_masked(_dtaGpio_bmsk);
        } else {
            __WRITE_GPIO(_pwrCtrlGpio_bmsk, state);
            gpio_set_dir_out_masked(_pwrCtrlGpio_bmsk);
        }
    }
#else
    TIME_CRITICAL void writeGpioOut(int state)
    {
        __WRITE_GPIO(_dtaGpio_bmsk, state);
    }

    TIME_CRITICAL void setGpioAsOutput(int state)
    {
        __WRITE_GPIO(_dtaGpio_bmsk, state);
        gpio_set_dir_out_masked(_dtaGpio_bmsk);
    }
#endif /* CONFIG_PWR_CTRL_ENABLED */

    void initDtaGpio(unsigned pin, bool pullUp)
    {
        assert(pin < NUM_BANK0_GPIOS);
        _dtaGpio_bmsk = (1UL << pin);

        __GPIO_INIT(pin);

        if (pullUp) {
            gpio_set_pulls(pin, true, false);
        } else {
            gpio_set_pulls(pin, false, false);
        }
        setupDtaGpio();
    }

#if CONFIG_PWR_CTRL_ENABLED
    void initPwrCtrlGpio(unsigned pin)
    {
        assert(pin < NUM_BANK0_GPIOS);
        _pwrCtrlGpio_bmsk = (1UL << pin);

        __GPIO_INIT(pin);
        setupPwrCtrlGpio(true);
    }

    uint32_t _pwrCtrlGpio_bmsk;
#endif

    uint32_t _dtaGpio_bmsk;
};

#undef __WRITE_GPIO
#undef __WRITE1_GPIO
#undef __WRITE0_GPIO
#undef __READ_GPIO
#undef __GPIO_INIT

#endif /* __OWNG_PICO_RP2040__ */

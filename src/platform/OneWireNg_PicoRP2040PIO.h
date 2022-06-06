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

#ifndef __OWNG_PICO_RP2040PIO__
#define __OWNG_PICO_RP2040PIO__

#include "OneWireNg.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "platform/rp2040/w1.pio.h"

/**
 * RP2040's PIO peripheral specific implementation of 1-wire bus activities:
 * reset, touch, parasite powering.
 */
class OneWireNg_PicoRP2040PIO: public OneWireNg
{
public:
    /**
     * OneWireNg 1-wire service for RP2040 platform (using PIO peripheral).
     *
     * Bus powering is supported via switching its GPIO to the high state.
     * In this case the GPIO servers as a voltage source for connected slaves
     * working in parasite powering configuration.
     *
     * @param pin RP2040's GPIO pin number used for bit-banging 1-wire bus.
     * @param pullUp If @c true configure internal pull-up resistor for the bus.
     */
    OneWireNg_PicoRP2040PIO(unsigned pin, bool pullUp)
    {
        assert(pin < 32);

#if (CONFIG_RP2040_PIO_NUM == 0)
        _pio = pio0;
#elif (CONFIG_RP2040_PIO_NUM == 1)
        _pio = pio1;
#else
# error "Invalid CONFIG_RP2040_PIO_NUM parameter. 0 or 1 expected."
#endif
        _sm = pio_claim_unused_sm(_pio, true);
        _pin = pin;

        powerBus(false);
        if (pullUp) gpio_pull_up(pin);

        pio_sm_set_enabled(_pio, _sm, false);

        _addrReset = pio_add_program(_pio, &w1_reset_program);
        _addrTouch0 = pio_add_program(_pio, &w1_touch0_program);
        _addrTouch1 = pio_add_program(_pio, &w1_touch1_program);

        uint sys_mhz = clock_get_hz(clk_sys) / 1000000;
        _divReset = w1_reset_cycle * sys_mhz;
        _divTouch0 = w1_touch0_cycle * sys_mhz;
        _divTouch1 = w1_touch1_cycle * sys_mhz;

        /* Prepare PIO configuration
         */
        _pioCfg = pio_get_default_sm_config();

        /* 1-bit side-set (required), output set */
        sm_config_set_sideset(&_pioCfg, 1, false, false);

        /* PINS, IN and SET configured to w1 pin */
        sm_config_set_set_pins(&_pioCfg, pin, 1);
        sm_config_set_in_pins(&_pioCfg, pin);
        sm_config_set_sideset_pins(&_pioCfg, pin);

        /* left-shift, IN threshold: 1 */
        sm_config_set_in_shift(&_pioCfg, false, false, 1);

        /* set the default config for the PIO SM */
        pio_sm_set_config(_pio, _sm, &_pioCfg);
    }

    /**
     * Transmit reset cycle on the 1-wire bus.
     */
    ErrorCode reset() {
        return (!pioRun(_divReset, _addrReset) ? EC_SUCCESS : EC_NO_DEVS);
    }

    /**
     * Bit touch.
     */
    int touchBit(int bit, bool power)
    {
        int res;

        /* pass type of power pull-up to the PIO SM */
        pio_sm_clear_fifos(_pio, _sm);
        if (power) {
            pio_sm_put(_pio, _sm, (bit ? w1_touch1_strong : w1_touch0_strong));
        } else {
            pio_sm_put(_pio, _sm, (bit ? w1_touch1_weak : w1_touch0_weak));
        }

        if (bit) {
            res = pioRun(_divTouch1, _addrTouch1);
        } else {
            res = pioRun(_divTouch0, _addrTouch0);
        }

        return (res != 0);
    }

    /**
     * Enable/disable direct voltage source provisioning on the 1-wire data bus.
     * Function always successes.
     */
    ErrorCode powerBus(bool on)
    {
        /* on: 1 (OUT-high), on: 0 (IN-high via pull-up) */
        gpio_init_mask(1 << _pin);
        gpio_set_dir(_pin, on);
        gpio_put(_pin, 1);

        return EC_SUCCESS;
    }

private:
    /**
     * Run w1 program on the PIO SM with given clock divider @c div.
     *
     * The program start at address @c addr and is already loaded
     * to the PIO's internal memory.
     */
    uint32_t pioRun(uint div, uint addr)
    {
        /* make sure w1 pin is connected to PIO SM */
        pio_gpio_init(_pio, _pin);

        /* move to program start */
        pio_sm_exec(_pio, _sm, pio_encode_jmp(addr));

        /* restart PIO SM */
        pio_sm_restart(_pio, _sm);
        pio_sm_clkdiv_restart(_pio, _sm);

        /* set new clock divider */
        _pio->sm[_sm].clkdiv = (div << PIO_SM0_CLKDIV_INT_LSB);

        /* start the program execution by PIO SM */
        pio_sm_set_enabled(_pio, _sm, true);

        /* wait until result will be ready */
        uint32_t res = pio_sm_get_blocking(_pio, _sm);

        /* stop PIO SM */
        pio_sm_set_enabled(_pio, _sm, false);

        return res;
    }

    PIO _pio;   /** PIO used */
    uint _sm;   /** PIO SM used */
    uint _pin;  /** w1 bus pin */

    /** PIO's w1 programs addresses */
    uint _addrReset;
    uint _addrTouch0;
    uint _addrTouch1;

    /** PIO clock dividers for w1 programs */
    uint _divReset;
    uint _divTouch0;
    uint _divTouch1;

    pio_sm_config _pioCfg;
};

#endif /* __OWNG_PICO_RP2040PIO__ */

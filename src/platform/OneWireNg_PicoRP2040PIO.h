/*
 * Copyright (c) 2022,2025 Piotr Stolarz
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

#include <string.h>  /* memcpy */
#include "OneWireNg.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "platform/rp2040/w1.pio.h"

#if CONFIG_RP2040_PIOSM_NUM_USED == 1
# define __SM_RESET _sm1
# define __SM_TOUCH _sm1
#elif CONFIG_RP2040_PIOSM_NUM_USED == 2
# define __SM_RESET _sm1
# define __SM_TOUCH _sm2
#else
# error "Invalid CONFIG_RP2040_PIOSM_NUM_USED"
#endif

/**
 * RP2040's PIO peripheral specific implementation of 1-wire bus activities:
 * reset, touch, parasite powering.
 *
 * @note The driver uses Pico SDK API to handle PIO activities. Since the SDK
 *     is part of Arduino framework, the driver may be used for both of these
 *     frameworks.
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
     * @param pioNum PIO number (0 or 1) used to execute 1-wire activities.
     */
    OneWireNg_PicoRP2040PIO(unsigned pin, bool pullUp, int pioNum = 0)
    {
        assert(pin < 32);
        assert(pioNum == 0 || pioNum == 1);
        _pin = pin;

        if (pioNum == 0)
            _pio = pio0;
        else
            _pio = pio1;

        _sm1 = pio_claim_unused_sm(_pio, true);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        _sm2 = pio_claim_unused_sm(_pio, true);
#endif

        _pioBound = false;
        _progOwner = true;
        _exeProg = INVALID_PROG;

        powerBus(false);
        if (pullUp) gpio_pull_up(pin);

        /* turn off PIO SM(s) */
        pio_sm_set_enabled(_pio, _sm1, false);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        pio_sm_set_enabled(_pio, _sm2, false);
#endif

        _addrs[RESET_STD]  = pio_add_program(_pio, &w1_reset_program);
        _addrs[TOUCH0_STD] = pio_add_program(_pio, &w1_touch0_program);
        _addrs[TOUCH1_STD] = pio_add_program(_pio, &w1_touch1_program);
#if CONFIG_OVERDRIVE_ENABLED
        _addrs[RESET_OD]   = _addrs[RESET_STD];
        _addrs[TOUCH0_OD]  = _addrs[TOUCH0_STD];
        _addrs[TOUCH1_OD]  = _addrs[TOUCH1_STD];
#endif
        uint sysMHz = clock_get_hz(clk_sys) / 1000000;
        _divs[RESET_STD]  = (w1_reset_cycle * sysMHz) / 10;
        _divs[TOUCH0_STD] = (w1_touch0_cycle * sysMHz) / 10;
        _divs[TOUCH1_STD] = (w1_touch1_cycle * sysMHz) / 10;
#if CONFIG_OVERDRIVE_ENABLED
        _divs[RESET_OD]   = (w1_reset_od_cycle * sysMHz) / 10;
        _divs[TOUCH0_OD]  = (w1_touch0_od_cycle * sysMHz) / 10;
        _divs[TOUCH1_OD]  = (w1_touch1_od_cycle * sysMHz) / 10;
#endif
        _wraps[RESET_STD]  = w1_reset_wrap_target;
        _wraps[TOUCH0_STD] = w1_touch0_wrap_target;
        _wraps[TOUCH1_STD] = w1_touch1_wrap_target;
#if CONFIG_OVERDRIVE_ENABLED
        _wraps[RESET_OD]   = _wraps[RESET_STD];
        _wraps[TOUCH0_OD]  = _wraps[TOUCH0_STD];
        _wraps[TOUCH1_OD]  = _wraps[TOUCH1_STD];
#endif
        /* Prepare PIO configuration
         */
        _pioCfg = pio_get_default_sm_config();

        /* 1-bit side-set (required), output set */
        sm_config_set_sideset(&_pioCfg, 1, false, false);

        /* PINS, IN and SET configured to w1 pin */
        sm_config_set_set_pins(&_pioCfg, pin, 1);
        sm_config_set_in_pins(&_pioCfg, pin);
        sm_config_set_sideset_pins(&_pioCfg, pin);

        /* left-shift, no-autopush, IN threshold: 1 */
        sm_config_set_in_shift(&_pioCfg, false, false, 1);

        /* set the default config for the PIO SM(s) */
        pio_sm_set_config(_pio, _sm1, &_pioCfg);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        pio_sm_set_config(_pio, _sm2, &_pioCfg);
#endif
    }

    /**
     * Make RP2040 1-wire service (using PIO peripheral) for a given @c pin
     * basing on already created 1-wire service as follows:
     * - Use the same PIO peripheral along with loaded programs as for the
     *   @c base service.
     * - New service will use its own 1 or 2 state machine(s), depending on
     *   @c CONFIG_RP2040_PIOSM_NUM_USED configuration.
     *
     * @warning While creating the service object using this constructor
     *     the @c base object MUST exist for the lifetime of the newly
     *     created object.
     *
     * @note The routine enables create up to 2 or 4 (depending on @c
     *     CONFIG_RP2040_PIOSM_NUM_USED configuration) 1-wire services
     *     handled by different pins for a single PIO peripheral.
     */
    OneWireNg_PicoRP2040PIO(
        unsigned pin, bool pullUp, const OneWireNg_PicoRP2040PIO& base)
    {
        assert(pin < 32);
        _pin = pin;
        _pio = base._pio;

        _sm1 = pio_claim_unused_sm(_pio, true);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        _sm2 = pio_claim_unused_sm(_pio, true);
#endif

        _pioBound = false;
        _progOwner = false;
        _exeProg = INVALID_PROG;

        powerBus(false);
        if (pullUp) gpio_pull_up(pin);

        /* turn off PIO SM(s) */
        pio_sm_set_enabled(_pio, _sm1, false);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        pio_sm_set_enabled(_pio, _sm2, false);
#endif

        memcpy(_addrs, base._addrs, sizeof(_addrs));
        memcpy(_divs, base._divs, sizeof(_divs));
        memcpy(_wraps, base._wraps, sizeof(_wraps));

        /* Prepare PIO configuration
         */
        _pioCfg = pio_get_default_sm_config();

        /* 1-bit side-set (required), output set */
        sm_config_set_sideset(&_pioCfg, 1, false, false);

        /* PINS, IN and SET configured to w1 pin */
        sm_config_set_set_pins(&_pioCfg, pin, 1);
        sm_config_set_in_pins(&_pioCfg, pin);
        sm_config_set_sideset_pins(&_pioCfg, pin);

        /* left-shift, no-autopush, IN threshold: 1 */
        sm_config_set_in_shift(&_pioCfg, false, false, 1);

        /* set the default config for the PIO SM(s) */
        pio_sm_set_config(_pio, _sm1, &_pioCfg);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        pio_sm_set_config(_pio, _sm2, &_pioCfg);
#endif
    }

    /**
     * Clean-up PIO resources while destroying the service.
     */
    ~OneWireNg_PicoRP2040PIO()
    {
        /* disclaim unused PIO SM(s) */
        pio_sm_set_enabled(_pio, _sm1, false);
        pio_sm_unclaim(_pio, _sm1);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
        pio_sm_set_enabled(_pio, _sm2, false);
        pio_sm_unclaim(_pio, _sm2);
#endif
        /*
         * Dispose the PIO programs only in case the object is in ownership
         * of them. Since OD uses the same PIO programs as STD mode, free
         * STD programs only.
         */
        if (_progOwner) {
            pio_remove_program(_pio, &w1_reset_program, _addrs[RESET_STD]);
            pio_remove_program(_pio, &w1_touch0_program, _addrs[TOUCH0_STD]);
            pio_remove_program(_pio, &w1_touch1_program, _addrs[TOUCH1_STD]);
            _progOwner = false;
        }
    }

    /**
     * Transmit reset cycle on the 1-wire bus.
     */
    ErrorCode reset()
    {
#if CONFIG_OVERDRIVE_ENABLED
        int progId = RESET_STD + (int)(_overdrive == true);
#else
        int progId = RESET_STD;
#endif
        return ((pioRun(progId, __SM_RESET) & 1) ? EC_NO_DEVS : EC_SUCCESS);
    }

    /**
     * Bit touch.
     */
    int touchBit(int bit, bool power)
    {
        static const uint16_t pwrpus[2][2] = {
            {w1_touch0_weak,   w1_touch1_weak},   /* weak pull-up */
            {w1_touch0_strong, w1_touch1_strong}  /* strong pull-up */
        };

        /* pass type of power pull-up to the PIO SM */
        pio_sm_clear_fifos(_pio, __SM_TOUCH);
        pio_sm_put(_pio, __SM_TOUCH, pwrpus[(uint)(power == true)][(uint)(bit != 0)]);

#if CONFIG_OVERDRIVE_ENABLED
        int progId = (bit ? TOUCH1_STD : TOUCH0_STD) + (int)(_overdrive == true);
#else
        int progId = (bit ? TOUCH1_STD : TOUCH0_STD);
#endif
        return (pioRun(progId, __SM_TOUCH) & 1);
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
        _pioBound = false;

        return EC_SUCCESS;
    }

protected:
    /** Run w1 program on the PIO SM. */
    uint32_t pioRun(int progId, uint sm)
    {
        /* bind w1-bus GPIO to PIO if needed */
        if (!_pioBound) {
            pio_gpio_init(_pio, _pin);
            _pioBound = true;
        }

        /*
         * Try to avoid some extra configuration if the lastly
         * executed program is the same as the requested one.
         */
        if (progId != _exeProg)
        {
            /* set wrap for the program */
            pio_sm_set_wrap(_pio, sm,
                _addrs[progId] + _wraps[progId],
                _addrs[progId] + _wraps[progId]);

            _exeProg = progId;
        }

        /* restart PIO SM */
        pio_sm_restart(_pio, sm);
        pio_sm_clkdiv_restart(_pio, sm);
        pio_sm_set_clkdiv_int_frac(_pio, sm, _divs[progId], 0);

        /* move to program start */
        pio_sm_exec(_pio, sm, pio_encode_jmp(_addrs[progId]));

        /* start the program execution by PIO SM */
        pio_sm_set_enabled(_pio, sm, true);

        /* wait until result will be ready */
        uint32_t res = pio_sm_get_blocking(_pio, sm);

        /* stop PIO SM */
        pio_sm_set_enabled(_pio, sm, false);

        return res;
    }

#if CONFIG_OVERDRIVE_ENABLED
    enum {
        RESET_STD = 0,
        RESET_OD,
        TOUCH0_STD,
        TOUCH0_OD,
        TOUCH1_STD,
        TOUCH1_OD,

        PROGS_NUM,
        INVALID_PROG = PROGS_NUM
    };
#else
    enum {
        RESET_STD = 0,
        TOUCH0_STD,
        TOUCH1_STD,

        PROGS_NUM,
        INVALID_PROG = PROGS_NUM
    };
#endif

    uint _pin;    /** w1 bus pin */
    PIO _pio;     /** PIO used */
    uint _sm1;    /** PIO SM 1 */
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
    uint _sm2;    /** PIO SM 2 */
#endif

    /** PIO SM(s) common config */
    pio_sm_config _pioCfg;

    /** Lastly executed program */
    int _exeProg;

    /** w1-bus GPIO bound to PIO flag */
    bool _pioBound;

    /** PIO programs ownership indicator */
    bool _progOwner;

    /** PIO's w1 programs addresses */
    uint _addrs[PROGS_NUM];

    /** PIO clock dividers for w1 programs */
    uint _divs[PROGS_NUM];

    /** Programs wrap addresses */
    uint _wraps[PROGS_NUM];
};

#undef __SM_TOUCH
#undef __SM_RESET

#endif /* __OWNG_PICO_RP2040PIO__ */

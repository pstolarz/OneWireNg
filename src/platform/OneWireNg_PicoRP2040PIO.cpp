/*
 * Copyright (c) 2022-2026 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */
#include "OneWireNg_Config.h"

#if (defined(PICO_BUILD) || defined(ARDUINO_ARCH_RP2040)) && CONFIG_RP2040_PIO_DRIVER

#include "platform/OneWireNg_PicoRP2040PIO.h"
#include "platform/rp2040/w1.pio.h"

#include <string.h>  /* memcpy */

#include "hardware/clocks.h"
#include "hardware/gpio.h"

#if CONFIG_RP2040_PIOSM_NUM_USED == 1
# define __SM_RESET _sm1
# define __SM_TOUCH _sm1
#elif CONFIG_RP2040_PIOSM_NUM_USED == 2
# define __SM_RESET _sm1
# define __SM_TOUCH _sm2
#else
# error "Invalid CONFIG_RP2040_PIOSM_NUM_USED"
#endif

OneWireNg_PicoRP2040PIO::OneWireNg_PicoRP2040PIO(
    unsigned pin, bool pullUp, int pioNum)
{
    assert(pin < 32);
    assert(pioNum < 0 || pioNum >= NUM_PIOS);

    _pin = pin;
    _pioNum = pioNum;

    switch (pioNum)
    {
    case 0:
        _pio = pio0;
        break;
    case 1:
        _pio = pio1;
        break;
#if NUM_PIOS > 2
    case 2:
        _pio = pio2;
        break;
#endif
    }
    _pioRefs[pioNum]++;

    _sm1 = pio_claim_unused_sm(_pio, true);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
    _sm2 = pio_claim_unused_sm(_pio, true);
#endif

    _pioBound = false;
    _exeProg = INVALID_PROG;

    powerBus(false);
    if (pullUp)
        gpio_pull_up(pin);

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

OneWireNg_PicoRP2040PIO::OneWireNg_PicoRP2040PIO(
        unsigned pin, bool pullUp, const OneWireNg_PicoRP2040PIO& base)
{
    assert(pin < 32);

    _pin = pin;
    _pioNum = base._pioNum;
    _pio = base._pio;
    _pioRefs[_pioNum]++;

    _sm1 = pio_claim_unused_sm(_pio, true);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
    _sm2 = pio_claim_unused_sm(_pio, true);
#endif

    _pioBound = false;
    _exeProg = INVALID_PROG;

    powerBus(false);
    if (pullUp)
        gpio_pull_up(pin);

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

OneWireNg_PicoRP2040PIO::~OneWireNg_PicoRP2040PIO()
{
    /* disclaim unused PIO SM(s) */
    pio_sm_set_enabled(_pio, _sm1, false);
    pio_sm_unclaim(_pio, _sm1);
#if CONFIG_RP2040_PIOSM_NUM_USED > 1
    pio_sm_set_enabled(_pio, _sm2, false);
    pio_sm_unclaim(_pio, _sm2);
#endif
    /*
     * Dispose the PIO programs in case the PIO peripheral has no more
     * references. Since OD uses the same PIO programs as STD mode, free
     * STD programs only.
     */
    if (--_pioRefs[_pioNum]) {
        pio_remove_program(_pio, &w1_reset_program, _addrs[RESET_STD]);
        pio_remove_program(_pio, &w1_touch0_program, _addrs[TOUCH0_STD]);
        pio_remove_program(_pio, &w1_touch1_program, _addrs[TOUCH1_STD]);
    }
}

OneWireNg::ErrorCode OneWireNg_PicoRP2040PIO::reset()
{
#if CONFIG_OVERDRIVE_ENABLED
    int progId = RESET_STD + (int)(_overdrive == true);
#else
    int progId = RESET_STD;
#endif
    return ((pioRun(progId, __SM_RESET) & 1) ? EC_NO_DEVS : EC_SUCCESS);
}

int OneWireNg_PicoRP2040PIO::touchBit(int bit, bool power)
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

OneWireNg::ErrorCode OneWireNg_PicoRP2040PIO::powerBus(bool on)
{
    /* on: 1 (OUT-high), on: 0 (IN-high via pull-up) */
    gpio_init_mask(1 << _pin);
    gpio_set_dir(_pin, on);
    gpio_put(_pin, 1);
    _pioBound = false;

    return EC_SUCCESS;
}

uint32_t OneWireNg_PicoRP2040PIO::pioRun(int progId, uint sm)
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

uint OneWireNg_PicoRP2040PIO::_pioRefs[NUM_PIOS] = {};

#endif

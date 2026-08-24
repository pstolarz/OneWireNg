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

#ifndef __OWNG_PICO_RP2040PIO__
#define __OWNG_PICO_RP2040PIO__

#include "OneWireNg.h"

#include "hardware/pio.h"

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
     * @param pioNum PIO number used to execute 1-wire activities (0-based).
     */
    OneWireNg_PicoRP2040PIO(unsigned pin, bool pullUp, int pioNum = 0);

    /**
     * Make RP2040 1-wire service (using PIO peripheral) for a given @c pin
     * basing on already created 1-wire service as follows:
     * - Use the same PIO peripheral along with loaded programs as for the
     *   @c base service.
     * - New service will use its own 1 or 2 state machine(s), depending on
     *   @c CONFIG_RP2040_PIOSM_NUM_USED configuration.
     *
     * @note The routine enables create up to 2 or 4 (depending on @c
     *     CONFIG_RP2040_PIOSM_NUM_USED configuration) 1-wire services
     *     handled by different pins for a single PIO peripheral.
     */
    OneWireNg_PicoRP2040PIO(
        unsigned pin, bool pullUp, const OneWireNg_PicoRP2040PIO& base);

    /**
     * Clean-up PIO resources while destroying the service.
     */
    ~OneWireNg_PicoRP2040PIO();

    /**
     * Transmit reset cycle on the 1-wire bus.
     */
    ErrorCode reset();

    /**
     * Bit touch.
     */
    int touchBit(int bit, bool power);

    /**
     * Enable/disable direct voltage source provisioning on the 1-wire data bus.
     * Function always successes.
     */
    ErrorCode powerBus(bool on);

protected:
    /** Run w1 program on the PIO SM. */
    uint32_t pioRun(int progId, uint sm);

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

    /** PIO number */
    int _pioNum;
    /** PIO used */
    PIO _pio;
    /** PIO ref counters */
    static uint _pioRefs[NUM_PIOS];

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

    /** PIO's w1 programs addresses */
    uint _addrs[PROGS_NUM];

    /** PIO clock dividers for w1 programs */
    uint _divs[PROGS_NUM];

    /** Programs wrap addresses */
    uint _wraps[PROGS_NUM];
};

#endif /* __OWNG_PICO_RP2040PIO__ */

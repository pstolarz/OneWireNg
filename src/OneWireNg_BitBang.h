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

#ifndef __OWNG_BITBANG__
#define __OWNG_BITBANG__

#include "OneWireNg.h"

/**
 * GPIO bit-banged implementation of 1-wire bus activities: reset, touch,
 * parasite powering.
 *
 * The class relies on virtual functions provided by derivative class to
 * perform platform specific GPIO operations. The platform specific class
 * shall provide:
 * - @ref readGpioIn(), @ref writeGpioOut(): read/write operations.
 * - @ref setGpioAsInput(), @ref setGpioAsOutput(): set GPIO working mode.
 */
class OneWireNg_BitBang: public OneWireNg
{
public:
    virtual ErrorCode reset();
    virtual int touchBit(int bit);

    /**
     * Enable/disable direct voltage source provisioning on the 1-wire data bus
     * parasitically powering connected slave devices. In case of open-drain
     * type of platform, where no power-control-GPIO has been configured,
     * the routine returns @c EC_UNSUPPORED, @c EC_SUCCESS.
     *
     * @see setupPwrCtrlGpio().
     */
    virtual ErrorCode powerBus(bool on);

protected:
    typedef enum
    {
        GPIO_DTA = 0,   /** 1-wire data GPIO */
        GPIO_CTRL_PWR   /** power-control-GPIO */
    } GpioType;

    /**
     * This class is intended to be inherited by specialized classes.
     *
     * @param openDrain Set to @c true if platform's GPIOs are of open-drain
     *    type (in the output mode).
     *
     * @see writeGpioOut().
     */
    OneWireNg_BitBang(bool openDrain = false)
    {
        _flgs.od = (openDrain != 0);
        _flgs.pwre = 0;
        _flgs.pwrp = 0;
        _flgs.pwrr = 0;
    }

    /**
     * For open-drain type of platform data bus GPIO can't serve as a voltage
     * source for parasitically power connected slaves. This routine enables /
     * disables power-control-GPIO (working in the output mode) controlling
     * power switching transistor providing the voltage source to the bus.
     * The GPIO is set to the low state in case the power is enabled on the
     * bus via @ref powerBus() routine and to the high state otherwise. The
     * logic may be inverted by setting @c reversePolarity to @c true.
     */
    void setupPwrCtrlGpio(bool on, bool reversePolarity = false)
    {
        if (on) {
            _flgs.pwrr = (reversePolarity != 0);
            setGpioAsOutput(GPIO_CTRL_PWR, (reversePolarity ? 0 : 1));
            _flgs.pwrp = 1;
        } else {
            _flgs.pwrp = 0;
        }
    }

    /**
     * Utility routine. Shall be called from inheriting class to initialize
     * data GPIO.
     */
    void setupDtaGpio() {
        setBus(1);
    }

    /**
     * Read input-mode @c gpio and return its state (0: low, 1: high).
     *
     * @note Currently the routine is called for data GPIO only (@c GPIO_DTA).
     *     However @c gpio parameter is passed to maintain common GPIO interface
     *     and reserve the parameter for possible future usage.
     */
    virtual int readGpioIn(GpioType gpio) = 0;

    /**
     * Write output-mode @c gpio with a given @c state (0: low, 1: high).
     *
     * Depending on a type of underlying platform (open-drain or non-open-drain
     * digital outputs) the high state is pulled-up by a resistor or directly
     * connected to a voltage source.
     *
     * @note The routine is called for data (@c GPIO_DTA) and power-control-GPIO
     *     (@c GPIO_CTRL_PWR). The latter case happens if and only if
     *     power-control-GPIO has been configured via @ref setupPwrCtrlGpio().
     */
    virtual void writeGpioOut(GpioType gpio, int state) = 0;

    /**
     * Set @c gpio in the input-mode.
     *
     * @note Currently the routine is called for data GPIO only (@c GPIO_DTA).
     *     However @c gpio parameter is passed to maintain common GPIO interface
     *     and reserve the parameter for possible future usage.
     */
    virtual void setGpioAsInput(GpioType gpio) = 0;

    /**
     * Set @c gpio in the output-mode with an initial @c state (0: low, 1: high).
     *
     * @note The function should guarantee no intermediate "blink" state between
     *     switching the GPIO into the output mode and setting the initial
     *     value on the pin. In case of problem with fulfilling such assumption
     *     there may be feasible to compile the library with @ref
     *     CONFIG_BUS_BLINK_PROTECTION, but such approach has its drawbacks too.
     *
     * @note The routine is called for data (@c GPIO_DTA) and power-control-GPIO
     *     (@c GPIO_CTRL_PWR). The latter case happens if and only if
     *     power-control-GPIO has been configured via @ref setupPwrCtrlGpio().
     */
    virtual void setGpioAsOutput(GpioType gpio, int state) = 0;

#ifdef CONFIG_OVERDRIVE_ENABLED
    /**
     * 1-wire touch-1 in overdrive mode is defined by 2-steps procedure:
     * 1. Set data bus low for 1 usec.
     * 2. Switch the bus into input and sample it at 2 usec max (measuring
     *    from setting the bus low in the previous step).
     *
     * The timings here are very strict and vulnerable for improper values.
     * In case the library is not able to properly bit-bang overdriven touch-1
     * via standard OneWireNg_BitBang's GPIO interface (read/write/set mode)
     * due to insufficient platform's CPU frequency causing too long timings,
     * the platform's GPIO implementation may override this method to directly
     * implement the touch-1 activity.
     *
     * @note The routine is called inside time critical section.
     *
     * @note Implementation shall respect @ref CONFIG_BUS_BLINK_PROTECTION
     *     configuration if the platform doesn't guarantee input to output GPIO
     *     switch with a desired initial state.
     */
    virtual int touch1Overdrive();
#endif

    /**
     * Set 1-wire data bus state: high (1) or low (0).
     */
    void setBus(int state)
    {
        if (state) {
#ifdef CONFIG_BUS_BLINK_PROTECTION
            writeGpioOut(GPIO_DTA, 1);
#endif
            setGpioAsInput(GPIO_DTA);
        } else {
            setGpioAsOutput(GPIO_DTA, 0);
        }
    }

    struct {
        unsigned od:   1;   /** open drain indicator */
        unsigned pwre: 1;   /** bus is powered indicator */
        unsigned pwrp: 1;   /** power-control-GPIO pin is valid */
        unsigned pwrr: 1;   /** power-control-GPIO works in reverse polarity */
    } _flgs;

#ifdef __TEST__
friend class OneWireNg_BitBang_Test;
#endif
};

#endif /* __OWNG_BITBANG__ */

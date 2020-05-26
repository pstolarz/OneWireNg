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

#include "OneWireNg_BitBang.h"

/*
 * Platform specific delay & interrupts API
 */
#ifdef ARDUINO
# include "Arduino.h"
# define delayUs(dly) delayMicroseconds(dly)
# define timeCriticalEnter() noInterrupts()
# define timeCriticalExit() interrupts()
#elif defined(__TEST__)
# include <unistd.h>
# define delayUs(dly) usleep(dly)
# define timeCriticalEnter()
# define timeCriticalExit()
#else
# error "Unsupported platform"
#endif

OneWireNg::ErrorCode OneWireNg_BitBang::reset()
{
    int presPulse;

    timeCriticalEnter();
    if (_flgs.pwre) powerBus(false);

#ifdef CONFIG_OVERDRIVE_ENABLED
    if (_overdrive)
    {
        /* overdrive mode */
        setBus(0);
        delayUs(68);    /* 53-80 us */
        setBus(1);
        delayUs(8);     /* presence-detect sample at 8-9 us */
        presPulse = readGpioIn(GPIO_DTA);
        timeCriticalExit();
        delayUs(40);
    } else
#endif
    {
        /* standard mode */
        setBus(0);
        timeCriticalExit();
        delayUs(480);   /* min 480 us */
        timeCriticalEnter();
        setBus(1);
        delayUs(70);    /* presence-detect sample at 68-75 us */
        presPulse = readGpioIn(GPIO_DTA);
        timeCriticalExit();
        delayUs(410);
    }
    return (presPulse ? EC_NO_DEVS : EC_SUCCESS);
}

int OneWireNg_BitBang::touchBit(int bit)
{
    int smpl = 0;

    timeCriticalEnter();
    if (_flgs.pwre) powerBus(false);

#ifdef CONFIG_OVERDRIVE_ENABLED
    if (_overdrive)
    {
        /* overdrive mode */
        if (bit != 0)
        {
            /* write-1 w/ sampling alias read */
            smpl = touch1Overdrive();
            timeCriticalExit();
            delayUs(7);
        } else
        {
            /* write-0 */
            setBus(0);
            delayUs(8);     /* 8-13 us */
            setBus(1);
            timeCriticalExit();
            delayUs(1);     /* 1-2 us */
        }
    } else
#endif
    {
        /* standard mode */
        if (bit != 0)
        {
            /* write-1 w/ sampling alias read */
            setBus(0);
            delayUs(5);
            setBus(1);
            delayUs(8);
            /* start sampling at 13us; max at 15 us */
            smpl = readGpioIn(GPIO_DTA);
            timeCriticalExit();
            delayUs(56);
        } else
        {
            /* write-0 */
            setBus(0);
            delayUs(60);    /* 60-120 us */
            setBus(1);
            timeCriticalExit();
            delayUs(10);    /* 5-15 us */
        }
    }
    return smpl;
}

OneWireNg::ErrorCode OneWireNg_BitBang::powerBus(bool on)
{
    if (!_flgs.od) {
        if (on) {
            setGpioAsOutput(GPIO_DTA, 1);
        } else {
            setGpioAsInput(GPIO_DTA);
        }
    } else
    if (_flgs.pwrp) {
        writeGpioOut(GPIO_CTRL_PWR, (_flgs.pwrr ? (on != 0) : !on));
    } else {
        return EC_UNSUPPORED;
    }
    _flgs.pwre = (on != 0);
    return EC_SUCCESS;
}

#ifdef CONFIG_OVERDRIVE_ENABLED
int OneWireNg_BitBang::touch1Overdrive()
{
    setBus(0);
    delayUs(1);
    setBus(1);
    delayUs(1);
    /* start sampling at 2 us */
    return readGpioIn(GPIO_DTA);
}
#endif

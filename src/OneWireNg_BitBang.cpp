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

/* Standard mode timings
 */
/* min. 480 us */
#define STD_RESET_LOW   480
/* reset high; presence-detect sampling: 68-75 us */
#define STD_RESET_SMPL  70
/* reset trailing high */
#define STD_RESET_END   410

/* write-0 low: 60-120 us */
#define STD_WRITE0_LOW  60
/* write-0 trailing high: 5-15 us */
#define STD_WRITE0_END  10

/* write-1 low */
#define STD_WRITE1_LOW  5
/* write-1 high; sampling max 15 us (low + high) */
#define STD_WRITE1_SMPL 8
/* write-1 trailing high */
#define STD_WRITE1_END  56

/* Overdrive mode timings
 */
/* reset low: 53-80 us */
#define OD_RESET_LOW    68
/* reset high; presence-detect sampling: 8-9 us */
#define OD_RESET_SMPL   8
/* reset high; trailing part */
#define OD_RESET_END    40

/* write-0 low: 8-13 us */
#define OD_WRITE0_LOW   8
/* write-0 trailing high: 1-2 us */
#define OD_WRITE0_END   1

/* write-1 low */
#define OD_WRITE1_LOW   0   /* <0: no delay, 0: delay(0)==NOP, >1: usec delay */
/* write-1 high; sampling max 2 us (low + high) */
#define OD_WRITE1_SMPL (-1) /* <0: no delay, 0: delay(0)==NOP, >1: usec delay */
/* write-1 trailing high */
#define OD_WRITE1_END   7

OneWireNg::ErrorCode OneWireNg_BitBang::reset()
{
    int presPulse;

    timeCriticalEnter();
    if (_flgs.pwre) powerBus(false);

#ifdef CONFIG_OVERDRIVE_ENABLED
    if (_overdrive)
    {
        /* Overdrive mode
         */
        setBus(0);
        delayUs(OD_RESET_LOW);
        setBus(1);
        delayUs(OD_RESET_SMPL);
        presPulse = readGpioIn(GPIO_DTA);
        timeCriticalExit();
        delayUs(OD_RESET_END);
    } else
#endif
    {
        /* Standard mode
         */
        setBus(0);
        timeCriticalExit();
        delayUs(STD_RESET_LOW);
        timeCriticalEnter();
        setBus(1);
        delayUs(STD_RESET_SMPL);
        presPulse = readGpioIn(GPIO_DTA);
        timeCriticalExit();
        delayUs(STD_RESET_END);
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
        /* Overdrive mode
         */
        if (bit != 0)
        {
            /* write-1 with sampling (alias read) */
            smpl = touch1Overdrive();
            timeCriticalExit();
            delayUs(OD_WRITE1_END);
        } else
        {
            /* write-0 */
            setBus(0);
            delayUs(OD_WRITE0_LOW);
            setBus(1);
            timeCriticalExit();
            delayUs(OD_WRITE0_END);
        }
    } else
#endif
    {
        /* Standard mode
         */
        if (bit != 0)
        {
            /* write-1 with sampling (alias read) */
            setBus(0);
            delayUs(STD_WRITE1_LOW);
            setBus(1);
            delayUs(STD_WRITE1_SMPL);
            smpl = readGpioIn(GPIO_DTA);
            timeCriticalExit();
            delayUs(STD_WRITE1_END);
        } else
        {
            /* write-0 */
            setBus(0);
            delayUs(STD_WRITE0_LOW);
            setBus(1);
            timeCriticalExit();
            delayUs(STD_WRITE0_END);
        }
    }
    return smpl;
}

#ifdef CONFIG_OVERDRIVE_ENABLED
int OneWireNg_BitBang::touch1Overdrive()
{
    setBus(0);
#if OD_WRITE1_LOW >= 0
    delayUs(OD_WRITE1_LOW);
#endif
    setBus(1);
#if OD_WRITE1_SMPL >= 0
    delayUs(OD_WRITE1_SMPL);
#endif
    return readGpioIn(GPIO_DTA);
}
#endif

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

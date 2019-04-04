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

#include "OneWireNg_BitBang.h"

/* delay & interrupts API */
#ifdef ARDUINO
# include "Arduino.h"
# define delayUs(dly) delayMicroseconds(dly)
# define enableInterrups() interrupts()
# define disableInterrupts() noInterrupts()
#else
#  ifndef __TEST__
#    warning "Interrupts API unsupported by the target platform"
#  endif
# include <unistd.h>
# define delayUs(dly) usleep(dly)
# define enableInterrups()
# define disableInterrupts()
#endif

bool OneWireNg_BitBang::reset()
{
    int presPulse;

    disableInterrupts();
    if (_flgs.pwre) powerBus(false);
    setBus(0);
    enableInterrups();
    delayUs(480);
    disableInterrupts();
    setBus(1);
    delayUs(90);
    presPulse = readGpioIn(GPIO_DTA);
    enableInterrups();
    delayUs(390);

    return !presPulse;
}

int OneWireNg_BitBang::touchBit(int bit)
{
    int smpl = 0;

    disableInterrupts();
    if (_flgs.pwre) powerBus(false);
    setBus(0);
    if (bit != 0)
    {
        /* write-1 w/ sampling alias read */
        delayUs(5);
        setBus(1);
        delayUs(8);
        /* start sampling at 13us */
        smpl = readGpioIn(GPIO_DTA);
        enableInterrups();
        delayUs(52);
    } else
    {
        /* write-0 */
        delayUs(65);
        setBus(1);
        enableInterrups();
        delayUs(5);
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

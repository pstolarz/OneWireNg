/*
 * Copyright (c) 2019,2021 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include "common.h"

class OneWireNg_BitBang_Test: OneWireNg_BitBang
{
private:
    OneWireNg_BitBang_Test() {}

    int readDtaGpioIn() { return 1; }
    void setDtaGpioAsInput() {}

#if CONFIG_PWR_CTRL_ENABLED
    void writeGpioOut(int state, GpioType gpio) {
        (void)state;
        (void)gpio;
    }
    void setGpioAsOutput(int state, GpioType gpio) {
        (void)state;
        (void)gpio;
    }
#else
    void writeGpioOut(int state) {
        (void)state;
    }
    void setGpioAsOutput(int state) {
        (void)state;
    }
#endif
};

int main(void)
{
    return 0;
}

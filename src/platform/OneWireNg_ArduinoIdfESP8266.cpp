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

#include "platform/OneWireNg_ArduinoIdfESP8266.h"
#ifdef IDF_VER
# include "sdkconfig.h"
#endif

/*
 * There have been identified and described problems with ESP platforms (both
 * ESP8266 and ESP32) while defining IRAM (TIME_CRITICAL) routines inside header
 * files, resulting with "dangerous relocation: l32r: literal placed after use"
 * linking phase errors. The solution to avoid this type of errors is to define
 * the IRAMed routines in separate compilation unit.
 */
#if defined(ARDUINO_ARCH_ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)
#include <assert.h>
#include <stdint.h>

#define ESP8266_REGISTER(addr) *((volatile uint32_t*)(0x60000000 + (addr)))

/* Control Registers (GPIO 0-15) */
#define REG_GPO    ESP8266_REGISTER(0x300) // GPIO_OUT R/W (Output Level)
#define REG_GPOS   ESP8266_REGISTER(0x304) // GPIO_OUT_SET WO
#define REG_GPOC   ESP8266_REGISTER(0x308) // GPIO_OUT_CLR WO
#define REG_GPE    ESP8266_REGISTER(0x30C) // GPIO_ENABLE R/W (Enable)
#define REG_GPES   ESP8266_REGISTER(0x310) // GPIO_ENABLE_SET WO
#define REG_GPEC   ESP8266_REGISTER(0x314) // GPIO_ENABLE_CLR WO
#define REG_GPI    ESP8266_REGISTER(0x318) // GPIO_IN RO (Read Input Level)
#define REG_GPIE   ESP8266_REGISTER(0x31C) // GPIO_STATUS R/W (Interrupt Enable)
#define REG_GPIES  ESP8266_REGISTER(0x320) // GPIO_STATUS_SET WO
#define REG_GPIEC  ESP8266_REGISTER(0x324) // GPIO_STATUS_CLR WO

/* Control Registers (GPIO 16) */
#define REG_GP16O  ESP8266_REGISTER(0x768)
#define REG_GP16E  ESP8266_REGISTER(0x774)
#define REG_GP16I  ESP8266_REGISTER(0x78C)

/* PIN Control Register (GPIO 0-15) */
#define REG_GPC(p) ESP8266_REGISTER(0x328 + ((p & 0xF) * 4))

/* PIN Control Register (GPIO 16) */
#define REG_GPC16  ESP8266_REGISTER(0x790)

/* PIN Function Registers (GPIO 0-15) */
#define REG_GPF0   ESP8266_REGISTER(0x834)
#define REG_GPF1   ESP8266_REGISTER(0x818)
#define REG_GPF2   ESP8266_REGISTER(0x838)
#define REG_GPF3   ESP8266_REGISTER(0x814)
#define REG_GPF4   ESP8266_REGISTER(0x83C)
#define REG_GPF5   ESP8266_REGISTER(0x840)
#define REG_GPF6   ESP8266_REGISTER(0x81C)
#define REG_GPF7   ESP8266_REGISTER(0x820)
#define REG_GPF8   ESP8266_REGISTER(0x824)
#define REG_GPF9   ESP8266_REGISTER(0x828)
#define REG_GPF10  ESP8266_REGISTER(0x82C)
#define REG_GPF11  ESP8266_REGISTER(0x830)
#define REG_GPF12  ESP8266_REGISTER(0x804)
#define REG_GPF13  ESP8266_REGISTER(0x808)
#define REG_GPF14  ESP8266_REGISTER(0x80C)
#define REG_GPF15  ESP8266_REGISTER(0x810)

static volatile uint32_t *_gpioToFn_tab[16] = {
    &REG_GPF0,  &REG_GPF1,  &REG_GPF2,  &REG_GPF3,
    &REG_GPF4,  &REG_GPF5,  &REG_GPF6,  &REG_GPF7,
    &REG_GPF8,  &REG_GPF9,  &REG_GPF10, &REG_GPF11,
    &REG_GPF12, &REG_GPF13, &REG_GPF14, &REG_GPF15
};
#define REG_GPF(p) (*_gpioToFn_tab[(p & 0xF)])

/* PIN Function Register (GPIO 16) */
#define REG_GPF16  ESP8266_REGISTER(0x7A0)

/* Function Bits (GPIO 0-15) */
#define BIT_GPFSOE   0 // Sleep OE
#define BIT_GPFSS    1 // Sleep Sel
#define BIT_GPFSPD   2 // Sleep Pulldown
#define BIT_GPFSPU   3 // Sleep Pullup
#define BIT_GPFFS0   4 // Function Select bit 0
#define BIT_GPFFS1   5 // Function Select bit 1
#define BIT_GPFPD    6 // Pulldown
#define BIT_GPFPU    7 // Pullup
#define BIT_GPFFS2   8 // Function Select bit 2

/* PIN Function Bits (GPIO 16) */
#define BIT_GP16FFS0 0 // Function Select bit 0
#define BIT_GP16FFS1 1 // Function Select bit 1
#define BIT_GP16FPD  3 // Pulldown
#define BIT_GP16FSPD 5 // Sleep Pulldown
#define BIT_GP16FFS2 6 // Function Select bit 2

#define BFD_GPFFS(f) \
    (((((f) & 4) != 0) << BIT_GPFFS2) | \
     ((((f) & 2) != 0) << BIT_GPFFS1) | \
     ((((f) & 1) != 0) << BIT_GPFFS0))

#define BFD_GP16FFS(f) \
    (((((f) & 4) != 0) << BIT_GP16FFS2) | \
     ((((f) & 2) != 0) << BIT_GP16FFS1) | \
     ((((f) & 1) != 0) << BIT_GP16FFS0))

#define GPFFS_GPIO_FN(p) \
    (((p) == 0 || (p) == 2 || (p) == 4 || (p) == 5) ? 0 : ((p) == 16) ? 1 : 3)


#define __READ_GPIO(gs) \
    ((*gs.inReg & gs.bmsk) != 0)

#define __WRITE0_GPIO(gs) REG_GPOC = gs.bmsk
#define __WRITE0_GPIO16() REG_GP16O &= ~(uint32_t)1

#define __WRITE1_GPIO(gs) REG_GPOS = gs.bmsk
#define __WRITE1_GPIO16() REG_GP16O |= (uint32_t)1

#define __WRITE_GPIO(gs, st) \
    if (gs.pin < 16) { \
        if (st) { __WRITE1_GPIO(gs); } else { __WRITE0_GPIO(gs); } \
    } else { \
        if (st) { __WRITE1_GPIO16(); } else { __WRITE0_GPIO16(); } \
    }

#define __GPIO_SET_INPUT(gs) REG_GPEC = gs.bmsk
#define __GPIO16_SET_INPUT() REG_GP16E &= ~(uint32_t)1

#define __GPIO_AS_INPUT(gs) \
    if (gs.pin < 16) { __GPIO_SET_INPUT(gs); } else { __GPIO16_SET_INPUT(); }

#define __GPIO_SET_OUTPUT(gs) REG_GPES = gs.bmsk
#define __GPIO16_SET_OUTPUT() REG_GP16E |= (uint32_t)1

#define __GPIO_AS_OUTPUT(gs) \
    if (gs.pin < 16) { __GPIO_SET_OUTPUT(gs); } else { __GPIO16_SET_OUTPUT(); }

/*
 * Set mode to GPIO.
 * SOURCE -> GPIO_DATA,
 * DRIVER -> NORMAL,
 * INT_TYPE -> DISABLED.
 */
static void pinInit(uint8_t pin)
{
    if (pin < 16) {
        REG_GPF(pin) = BFD_GPFFS(GPFFS_GPIO_FN(pin));
        REG_GPC(pin) = 0;
    } else if (pin == 16) {
        REG_GPF16 = BFD_GP16FFS(GPFFS_GPIO_FN(pin));
        REG_GPC16 = 0;
    }
}

TIME_CRITICAL int OneWireNg_ArduinoIdfESP8266::readDtaGpioIn()
{
    return __READ_GPIO(_dtaGpio);
}

TIME_CRITICAL void OneWireNg_ArduinoIdfESP8266::setDtaGpioAsInput()
{
    __GPIO_AS_INPUT(_dtaGpio);
}

#if CONFIG_PWR_CTRL_ENABLED
TIME_CRITICAL void OneWireNg_ArduinoIdfESP8266::writeGpioOut(
    int state, GpioType gpio)
{
    if (gpio == GPIO_DTA) {
        __WRITE_GPIO(_dtaGpio, state);
    } else {
        __WRITE_GPIO(_pwrCtrlGpio, state);
    }
}

TIME_CRITICAL void OneWireNg_ArduinoIdfESP8266::setGpioAsOutput(
    int state, GpioType gpio)
{
    if (gpio == GPIO_DTA) {
        __WRITE_GPIO(_dtaGpio, state);
        __GPIO_AS_OUTPUT(_dtaGpio);
    } else {
        __WRITE_GPIO(_pwrCtrlGpio, state);
        __GPIO_AS_OUTPUT(_pwrCtrlGpio);
    }
}
#else
TIME_CRITICAL void OneWireNg_ArduinoIdfESP8266::writeGpioOut(int state)
{
    __WRITE_GPIO(_dtaGpio, state);
}

TIME_CRITICAL void OneWireNg_ArduinoIdfESP8266::setGpioAsOutput(int state)
{
    __WRITE_GPIO(_dtaGpio, state);
    __GPIO_AS_OUTPUT(_dtaGpio);
}
#endif /* CONFIG_PWR_CTRL_ENABLED */

#if CONFIG_OVERDRIVE_ENABLED
TIME_CRITICAL int OneWireNg_ArduinoIdfESP8266::touch1Overdrive()
{
    if (_dtaGpio.pin < 16)
    {
        __WRITE0_GPIO(_dtaGpio);
        __GPIO_SET_OUTPUT(_dtaGpio);

        /* speed up low-to-high transition */
        __WRITE1_GPIO(_dtaGpio);
        __GPIO_SET_INPUT(_dtaGpio);
    } else
    {
        __WRITE0_GPIO16();
        __GPIO16_SET_OUTPUT();

        /* speed up low-to-high transition */
        __WRITE1_GPIO16();
        __GPIO16_SET_INPUT();
    }
    return __READ_GPIO(_dtaGpio);
}
#endif

void OneWireNg_ArduinoIdfESP8266::initDtaGpio(unsigned pin, bool pullUp)
{
    /* only pins < 16 can be configured with internal pull-up */
    assert(pullUp ? pin < 16 : pin <= 16);

#if CONFIG_BITBANG_DELAY_CCOUNT
    /* retrieve CPU frequency (for clock-count bit-banging mode) */
    ccntUpdateCpuFreqMHz();
#endif

    _dtaGpio.pin = pin;
    _dtaGpio.bmsk = (pin < 16 ? (uint32_t)(1UL << pin) : 1);
    _dtaGpio.inReg = (pin < 16 ? &REG_GPI : &REG_GP16I);

    pinInit(pin);
    if (pullUp) {
        /* enable pull-up */
        REG_GPF(pin) |= (1 << BIT_GPFPU);
    }
    setupDtaGpio();
}

#if CONFIG_PWR_CTRL_ENABLED
void OneWireNg_ArduinoIdfESP8266::initPwrCtrlGpio(unsigned pin)
{
    assert(pin <= 16);

    _pwrCtrlGpio.pin = pin;
    _pwrCtrlGpio.bmsk = (pin < 16 ? (uint32_t)(1UL << pin) : 1);

    pinInit(pin);
    setupPwrCtrlGpio(true);
}
#endif

#undef __GPIO_AS_OUTPUT
#undef __GPIO16_SET_OUTPUT
#undef __GPIO_SET_OUTPUT
#undef __GPIO_AS_INPUT
#undef __GPIO16_SET_INPUT
#undef __GPIO_SET_INPUT
#undef __WRITE_GPIO
#undef __WRITE1_GPIO16
#undef __WRITE1_GPIO
#undef __WRITE0_GPIO16
#undef __WRITE0_GPIO
#undef __READ_GPIO

#endif /* ESP8266 */

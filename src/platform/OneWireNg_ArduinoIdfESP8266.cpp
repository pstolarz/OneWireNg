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

#ifdef ARDUINO
# include "Arduino.h"
#else
/* ESP-IDF */
# include <stdint.h>

# define ESP8266_REG(addr) *((volatile uint32_t*)(0x60000000 + (addr)))

/* GPIO (0-15) Control Registers */
# define GPO    ESP8266_REG(0x300) /* GPIO_OUT R/W (Output Level) */
# define GPOS   ESP8266_REG(0x304) /* GPIO_OUT_SET WO */
# define GPOC   ESP8266_REG(0x308) /* GPIO_OUT_CLR WO */
# define GPE    ESP8266_REG(0x30C) /* GPIO_ENABLE R/W (Enable) */
# define GPES   ESP8266_REG(0x310) /* GPIO_ENABLE_SET WO */
# define GPEC   ESP8266_REG(0x314) /* GPIO_ENABLE_CLR WO */
# define GPI    ESP8266_REG(0x318) /* GPIO_IN RO (Read Input Level) */
# define GPIE   ESP8266_REG(0x31C) /* GPIO_STATUS R/W (Interrupt Enable) */
# define GPIES  ESP8266_REG(0x320) /* GPIO_STATUS_SET WO */
# define GPIEC  ESP8266_REG(0x324) /* GPIO_STATUS_CLR WO */

/* GPIO (0-15) PIN Control Bits */
# define GPCWE  10 /* WAKEUP_ENABLE (can be 1 only when INT_TYPE is high or low) */
# define GPCI   7  /* INT_TYPE (3bits) 0:disable,1:rising,2:falling,3:change,4:low,5:high */
# define GPCD   2  /* DRIVER 0:normal,1:open drain */
# define GPCS   0  /* SOURCE 0:GPIO_DATA,1:SigmaDelta */

/* GPIO (0-15) PIN Control Register */
# define GPC(p) ESP8266_REG(0x328 + ((p & 0xF) * 4))

/* GPIO (0-15) Function Bits */
# define GPFSOE 0 /* Sleep OE */
# define GPFSS  1 /* Sleep Sel */
# define GPFSPD 2 /* Sleep Pulldown */
# define GPFSPU 3 /* Sleep Pullup */
# define GPFFS0 4 /* Function Select bit 0 */
# define GPFFS1 5 /* Function Select bit 1 */
# define GPFPD  6 /* Pulldown */
# define GPFPU  7 /* Pullup */
# define GPFFS2 8 /* Function Select bit 2 */

/* GPIO (0-15) PIN Function Registers */
# define GPF0   ESP8266_REG(0x834)
# define GPF1   ESP8266_REG(0x818)
# define GPF2   ESP8266_REG(0x838)
# define GPF3   ESP8266_REG(0x814)
# define GPF4   ESP8266_REG(0x83C)
# define GPF5   ESP8266_REG(0x840)
# define GPF6   ESP8266_REG(0x81C)
# define GPF7   ESP8266_REG(0x820)
# define GPF8   ESP8266_REG(0x824)
# define GPF9   ESP8266_REG(0x828)
# define GPF10  ESP8266_REG(0x82C)
# define GPF11  ESP8266_REG(0x830)
# define GPF12  ESP8266_REG(0x804)
# define GPF13  ESP8266_REG(0x808)
# define GPF14  ESP8266_REG(0x80C)
# define GPF15  ESP8266_REG(0x810)

static volatile uint32_t *esp8266_gpioToFn[16] = {
    &GPF0, &GPF1, &GPF2,  &GPF3,  &GPF4,  &GPF5,  &GPF6,  &GPF7,
    &GPF8, &GPF9, &GPF10, &GPF11, &GPF12, &GPF13, &GPF14, &GPF15
};
# define GPF(p) (*esp8266_gpioToFn[(p & 0xF)])

# define GPFFS(f) \
    (((((f) & 4) != 0) << GPFFS2) | \
     ((((f) & 2) != 0) << GPFFS1) | \
     ((((f) & 1) != 0) << GPFFS0))

# define GPFFS_GPIO(p) \
    (((p) == 0 || (p) == 2 || (p) == 4 || (p) == 5) ? \
     0 : ((p) == 16) ? 1 : 3)

/* GPIO 16 Control Registers */
# define GP16O  ESP8266_REG(0x768)
# define GP16E  ESP8266_REG(0x774)
# define GP16I  ESP8266_REG(0x78C)

/* GPIO 16 PIN Control Register */
# define GP16C  ESP8266_REG(0x790)
# define GPC16  GP16C

/* GPIO 16 PIN Function Bits */
# define GP16FFS0 0 /* Function Select bit 0 */
# define GP16FFS1 1 /* Function Select bit 1 */
# define GP16FPD  3 /* Pulldown */
# define GP16FSPD 5 /* Sleep Pulldown */
# define GP16FFS2 6 /* Function Select bit 2 */
# define GP16FFS(f) (((f) & 0x03) | (((f) & 0x04) << 4))

/* GPIO 16 PIN Function Register */
# define GP16F  ESP8266_REG(0x7A0)
# define GPF16  GP16F

# define INPUT             0x00
# define OUTPUT            0x01
# define INPUT_PULLUP      0x02

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin < 16) {
        if (mode == OUTPUT)
        {
            /* set mode to GPIO */
            GPF(pin) = GPFFS(GPFFS_GPIO(pin));
            /* SOURCE(GPIO) | DRIVER(NORMAL) |
               INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED) */
            GPC(pin) = (GPC(pin) & (0xF << GPCI));
            /* enable */
            GPES = (1 << pin);
        } else if (mode == INPUT || mode == INPUT_PULLUP)
        {
            /* set mode to GPIO */
            GPF(pin) = GPFFS(GPFFS_GPIO(pin));
            /* disable */
            GPEC = (1 << pin);
            /* SOURCE(GPIO) | DRIVER(OPEN_DRAIN) |
               INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED) */
            GPC(pin) = (GPC(pin) & (0xF << GPCI)) | (1 << GPCD);
            if (mode == INPUT_PULLUP) {
                /* enable  pull-up */
                GPF(pin) |= (1 << GPFPU);
            }
        }
    } else if (pin == 16)
    {
        /* set mode to GPIO */
        GPF16 = GP16FFS(GPFFS_GPIO(pin));
        GPC16 = 0;
        if (mode == INPUT) {
            GP16E &= ~1;
        } else if (mode == OUTPUT) {
            GP16E |= 1;
        }
    }
}
#endif /* ESP-IDF */

#define __READ_GPIO(gs) \
    ((*gs.inReg & gs.bmsk) != 0)

#define __WRITE0_GPIO(gs) GPOC = gs.bmsk
#define __WRITE0_GPIO16() GP16O &= ~(uint32_t)1

#define __WRITE1_GPIO(gs) GPOS = gs.bmsk
#define __WRITE1_GPIO16() GP16O |= (uint32_t)1

#define __WRITE_GPIO(gs, st) \
    if (gs.pin < 16) { \
        if (st) { __WRITE1_GPIO(gs); } else { __WRITE0_GPIO(gs); } \
    } else { \
        if (st) { __WRITE1_GPIO16(); } else { __WRITE0_GPIO16(); } \
    }

#define __GPIO_SET_INPUT(gs) GPEC = gs.bmsk
#define __GPIO16_SET_INPUT() GP16E &= ~(uint32_t)1

#define __GPIO_AS_INPUT(gs) \
    if (gs.pin < 16) { __GPIO_SET_INPUT(gs); } else { __GPIO16_SET_INPUT(); }

#define __GPIO_SET_OUTPUT(gs) GPES = gs.bmsk
#define __GPIO16_SET_OUTPUT() GP16E |= (uint32_t)1

#define __GPIO_AS_OUTPUT(gs) \
    if (gs.pin < 16) { __GPIO_SET_OUTPUT(gs); } else { __GPIO16_SET_OUTPUT(); }

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
    _dtaGpio.inReg = (pin < 16 ? &GPI : &GP16I);

    pinMode(pin, (pullUp ? INPUT_PULLUP : INPUT));
    setupDtaGpio();
}

#if CONFIG_PWR_CTRL_ENABLED
void OneWireNg_ArduinoIdfESP8266::initPwrCtrlGpio(unsigned pin)
{
    assert(pin <= 16);

    _pwrCtrlGpio.pin = pin;
    _pwrCtrlGpio.bmsk = (pin < 16 ? (uint32_t)(1UL << pin) : 1);

    pinMode(pin, OUTPUT);
    setupPwrCtrlGpio(true);
}
#endif

#endif /* ESP8266 */

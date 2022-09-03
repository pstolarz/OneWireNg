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

/**
 * Dallas family thermometers access example (Pico SDK).
 *
 * Required configuration:
 * - @c CONFIG_SEARCH_ENABLED if @c SINGLE_SENSOR is not defined,
 * - @c CONFIG_PWR_CTRL_ENABLED if @c PWR_CTRL_PIN is defined.
 */
#include <stdio.h>
#include "pico/stdio.h"

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"
#include "platform/Platform_Delay.h"

/*
 * 1-wire bus pin number.
 */
#ifndef OW_PIN
# define OW_PIN         13
#endif

/*
 * If defined: sensors powered parasitically.
 */
//#define PARASITE_POWER

/*
 * If defined: only one sensor device is allowed to be connected to the bus.
 * The library may be configured with 1-wire search activity disabled to
 * reduce its footprint.
 */
//#define SINGLE_SENSOR

/*
 * The parameter specific type of power provisioning for parasitically powered
 * sensors:
 * - Not defined: power provided by bus pin.
 * - Defined: power provided by a switching transistor and controlled by the
 *   pin number specified by the parameter.
 */
//#define PWR_CTRL_PIN    9

/*
 * If defined: set permanent, common resolution for all sensors on the bus.
 * Resolution may vary from 9 to 12 bits.
 */
//#define COMMON_RES      (DSTherm::RES_12_BIT)

#if !defined(SINGLE_SENSOR) && !CONFIG_SEARCH_ENABLED
# error "CONFIG_SEARCH_ENABLED is required for non SINGLE_SENSOR setup"
#endif

#if defined(PWR_CTRL_PIN) && !CONFIG_PWR_CTRL_ENABLED
# error "CONFIG_PWR_CTRL_ENABLED is required if PWR_CTRL_PIN is defined"
#endif

#if (CONFIG_MAX_SEARCH_FILTERS > 0)
static_assert(CONFIG_MAX_SEARCH_FILTERS >= DSTherm::SUPPORTED_SLAVES_NUM,
    "CONFIG_MAX_SEARCH_FILTERS too small");
#endif

#ifdef PARASITE_POWER
# define PARASITE_POWER_ARG true
#else
# define PARASITE_POWER_ARG false
#endif

static Placeholder<OneWireNg_CurrentPlatform> _ow;

/* returns false if not supported */
static bool printId(const OneWireNg::Id& id)
{
    const char *name = DSTherm::getFamilyName(id);

    for (size_t i = 0; i < sizeof(OneWireNg::Id); i++)
        printf("%s%02x", (!i ? "" : ":"), id[i]);

    if (name)
        printf(" -> %s", name);

    printf("\n");

    return (name != NULL);
}

static void printScratchpad(const DSTherm::Scratchpad& scrpd)
{
    const uint8_t *scrpd_raw = scrpd.getRaw();

    printf("  Scratchpad:");
    for (size_t i = 0; i < DSTherm::Scratchpad::LENGTH; i++)
        printf("%c%02x", (!i ? ' ' : ':'), scrpd_raw[i]);


    printf("; Th:%d; Tl:%d; Resolution:%d",
        scrpd.getTh(), scrpd.getTl(),
        9 + (int)(scrpd.getResolution() - DSTherm::RES_9_BIT));

    long temp = scrpd.getTemp();
    printf("; Temp:");
    if (temp < 0) {
        temp = -temp;
        printf("-");
    }
    printf("%d.%d C\n", (int)temp / 1000, (int)temp % 1000);
}

void setup()
{
    stdio_init_all();

#ifdef PWR_CTRL_PIN
    new (&_ow) OneWireNg_CurrentPlatform(OW_PIN, PWR_CTRL_PIN, false);
#else
    new (&_ow) OneWireNg_CurrentPlatform(OW_PIN, false);
#endif
    DSTherm drv(*_ow);

#if (CONFIG_MAX_SEARCH_FILTERS > 0)
    drv.filterSupportedSlaves();
#endif

#ifdef COMMON_RES
    /*
     * Set common resolution for all sensors.
     * Th, Tl (high/low alarm triggers) are set to 0.
     */
    drv.writeScratchpadAll(0, 0, COMMON_RES);

    /*
     * The configuration above is stored in volatile sensors scratchpad
     * memory and will be lost after power unplug. Therefore store the
     * configuration permanently in sensors EEPROM.
     */
    drv.copyScratchpadAll(PARASITE_POWER_ARG);
#endif
}

void loop()
{
    static Placeholder<DSTherm::Scratchpad> _scrpd;
    DSTherm drv(*_ow);

    /* convert temperature on all sensors connected... */
    drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER_ARG);

#ifdef SINGLE_SENSOR
    /* single sensor expected */
    OneWireNg::ErrorCode ec = drv.readScratchpadSingle(&_scrpd);
    if (ec == OneWireNg::EC_SUCCESS) {
        printId((*_scrpd).getId());
        printScratchpad(*_scrpd);
    } else if (ec == OneWireNg::EC_CRC_ERROR)
        printf("  CRC error.\n");
#else
    /* read sensors one-by-one */
    for (const auto& id: *_ow) {
        if (printId(id)) {
            if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS)
                printScratchpad(*_scrpd);
            else
                printf("  Read scratchpad error.\n");
        }
    }
#endif

    printf("----------\n");
    delayMs(1000);
}

int main()
{
    setup();
    for (;;)
        loop();

    return 0;
}

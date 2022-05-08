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
 * Dallas family thermometers access example (ESP-IDF).
 */
#include <stdio.h>

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"
#include "platform/Platform_Delay.h"

#define OW_PIN          13

/*
 * Set to true for parasitically powered sensors.
 */
#define PARASITE_POWER  false

/*
 * Uncomment for single sensor setup.
 *
 * With this scenario only one sensor device is allowed to be connected
 * to the bus. The library may be configured with 1-wire search activity
 * disabled to reduce its footprint.
 */
//#define SINGLE_SENSOR

/*
 * Uncomment for power provided by a switching
 * transistor and controlled by this pin.
 */
//#define PWR_CTRL_PIN    9

/*
 * Uncomment to set permanent, common resolution for all
 * sensors on the bus. Resolution may vary from 9 to 12 bits.
 */
//#define COMMON_RES      (DSTherm::RES_12_BIT)

#if !defined(SINGLE_SENSOR) && !defined(CONFIG_SEARCH_ENABLED)
# error "CONFIG_SEARCH_ENABLED is required for non SINGLE_SENSOR setup"
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
#ifdef PWR_CTRL_PIN
# ifndef CONFIG_PWR_CTRL_ENABLED
#  error "CONFIG_PWR_CTRL_ENABLED needs to be enabled"
# endif
    new (&_ow) OneWireNg_CurrentPlatform(OW_PIN, PWR_CTRL_PIN, false);
#else
    new (&_ow) OneWireNg_CurrentPlatform(OW_PIN, false);
#endif
    DSTherm drv(_ow);

#if (CONFIG_MAX_SRCH_FILTERS > 0)
    static_assert(CONFIG_MAX_SRCH_FILTERS >= DSTherm::SUPPORTED_SLAVES_NUM,
        "CONFIG_MAX_SRCH_FILTERS too small");

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
    drv.copyScratchpadAll(PARASITE_POWER);
#endif
}

void loop()
{
    OneWireNg& ow = _ow;
    DSTherm drv(ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;

    /* convert temperature on all sensors connected... */
    drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER);

#ifdef SINGLE_SENSOR
    /* single sensor expected */
    OneWireNg::Id id;

    OneWireNg::ErrorCode ec = ow.readSingleId(id);
    if (ec == OneWireNg::EC_SUCCESS) {
        if (printId(id)) {
            if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS)
                printScratchpad(_scrpd);
            else
                printf("  Invalid device.\n");
        }
    } else if (ec == OneWireNg::EC_CRC_ERROR) {
        printf("  Invalid CRC. "
            "Possibly more than one device connected to the bus.\n");
    }
#else
    /* read sensors one-by-one */
    for (const auto& id: ow) {
        if (printId(id)) {
            if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS)
                printScratchpad(_scrpd);
            else
                printf("  Invalid device\n");
        }
    }
#endif

    printf("----------\n");
    delayMs(1000);
}

extern "C" void app_main()
{
    setup();
    for (;;)
        loop();
}

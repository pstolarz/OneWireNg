/*
 * Copyright (c) 2022,2024 Piotr Stolarz
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
 * Dallas family thermometers access example (Mbed OS).
 *
 * Required configuration:
 * - @c CONFIG_SEARCH_ENABLED for non single sensor setup,
 * - @c CONFIG_PWR_CTRL_ENABLED if @c MBED_CONF_APP_PWR_CTRL_PIN is configured.
 */
#include <stdio.h>

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"
#include "platform/Platform_Delay.h"

#ifndef MBED_CONF_APP_OW_PIN
# error "MBED_CONF_APP_OW_PIN is required"
#endif

#if !MBED_CONF_APP_SINGLE_SENSOR && !CONFIG_SEARCH_ENABLED
# error "CONFIG_SEARCH_ENABLED is required for non signle sensor setup"
#endif

#if defined(MBED_CONF_APP_PWR_CTRL_PIN) && !CONFIG_PWR_CTRL_ENABLED
# error "CONFIG_PWR_CTRL_ENABLED is required if MBED_CONF_APP_PWR_CTRL_PIN is configured"
#endif

#if defined(MBED_CONF_APP_COMMON_RES) && \
    ((MBED_CONF_APP_COMMON_RES < 9) || (MBED_CONF_APP_COMMON_RES > 12))
# error "Invalid MBED_CONF_APP_COMMON_RES"
#endif

#if (CONFIG_MAX_SEARCH_FILTERS > 0)
static_assert(CONFIG_MAX_SEARCH_FILTERS >= DSTherm::SUPPORTED_SLAVES_NUM,
    "CONFIG_MAX_SEARCH_FILTERS too small");
#endif

#if MBED_CONF_APP_PARASITE_POWER
# define PARASITE_POWER_ARG true
#else
# define PARASITE_POWER_ARG false
#endif

static Placeholder<OneWireNg_CurrentPlatform> ow;

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

    long temp = scrpd.getTemp2();
    printf("; Temp:");
    if (temp < 0) {
        temp = -temp;
        printf("-");
    }
    printf("%d.%04d C\n", (int)temp / 16, (10000 * ((int)temp % 16)) / 16);
}

void setup()
{
#ifdef MBED_CONF_APP_PWR_CTRL_PIN
    new (&ow) OneWireNg_CurrentPlatform(
        MBED_CONF_APP_OW_PIN, MBED_CONF_APP_PWR_CTRL_PIN, false);
#else
    new (&ow) OneWireNg_CurrentPlatform(MBED_CONF_APP_OW_PIN, false);
#endif
    DSTherm drv(ow);

#if (CONFIG_MAX_SEARCH_FILTERS > 0)
    drv.filterSupportedSlaves();
#endif

#ifdef MBED_CONF_APP_COMMON_RES
    /*
     * Set common resolution for all sensors.
     * Th, Tl (high/low alarm triggers) are set to 0.
     */
    drv.writeScratchpadAll(0, 0,
        (DSTherm::Resolution)(DSTherm::RES_9_BIT + (MBED_CONF_APP_COMMON_RES - 9)));

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
    DSTherm drv(ow);

    /* convert temperature on all sensors connected... */
    drv.convertTempAll(DSTherm::MAX_CONV_TIME, PARASITE_POWER_ARG);

#if MBED_CONF_APP_SINGLE_SENSOR
    /* single sensor environment */

    /*
     * Scratchpad placeholder is static to allow reuse of the associated
     * sensor id while reissuing readScratchpadSingle() calls.
     * Note, due to its storage class the placeholder is zero initialized.
     */
    static Placeholder<DSTherm::Scratchpad> scrpd;

    OneWireNg::ErrorCode ec = drv.readScratchpadSingle(scrpd);
    if (ec == OneWireNg::EC_SUCCESS) {
        printId(scrpd->getId());
        printScratchpad(scrpd);
    } else if (ec == OneWireNg::EC_CRC_ERROR)
        printf("  CRC error.\n");
#else
    /* read sensors one-by-one */
    Placeholder<DSTherm::Scratchpad> scrpd;

    for (const auto& id: *ow) {
        if (printId(id)) {
            if (drv.readScratchpad(id, scrpd) == OneWireNg::EC_SUCCESS)
                printScratchpad(scrpd);
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

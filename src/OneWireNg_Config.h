/*
 * Copyright (c) 2019-2021 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __OWNG_CONFIG__
#define __OWNG_CONFIG__

#ifdef OWNG_CONFIG_FILE
/* use user defined config file instead of this one */
# include OWNG_CONFIG_FILE
#elif !defined(OWNG_DISABLE_DEFAULT_CONFIG)

/**
 * The parameter controls the way the 1-wire bus is powered for parasitically
 * connected devices.
 *
 * If not configured, the bus is powered by setting the 1-wire data GPIO into
 * the high state and therefore connecting GPIO's voltage source directly to
 * the bus. The mode is feasible for non open-drain type of platforms where
 * a GPIO may provide sufficient power to connected slaves.
 *
 * If configured, the library provides an additional mode with a power-control-GPIO
 * (working in the output mode) controlling a power switching transistor
 * providing an external voltage source to the bus. The GPIO is set to the low
 * state in case the power is enabled on the bus via @ref OneWireNg::powerBus()
 * and to the high state otherwise (the logic may be reversed by
 * @c CONFIG_PWR_CTRL_REV_POLARITY parameter).
 */
//#define CONFIG_PWR_CTRL_ENABLED

/**
 * Reverse power-control-GPIO logic controlling the power switching transistor.
 *
 * @see CONFIG_PWR_CTRL_ENABLED.
 */
//#define CONFIG_PWR_CTRL_REV_POLARITY

/**
 * Maximum number of family codes used for search filtering.
 * If not defined or 0 - filtering disabled.
 */
#define CONFIG_MAX_SRCH_FILTERS 10

/**
 * Overdrive (high-speed) mode enabled.
 */
//#define CONFIG_OVERDRIVE_ENABLED

/**
 * Type of algorithm used for CRC-8/MAXIM calculation.
 *
 * The macro may be defined as:
 * @c CRC8_BASIC: Basic method. No memory tables used. This method is about 8
 *     times slower than the tabled method but no extra memory is used.
 * @c CRC8_TAB_16LH: 2x16 elements table, 1 byte each.
 */
#define CONFIG_CRC8_ALGO CRC8_TAB_16LH

/**
 * Enable CRC-16/ARC.
 */
//#define CONFIG_CRC16_ENABLED

/**
 * Type of algorithm used for CRC-16/ARC calculation.
 * Valid only if CRC-16 is enabled via @ref CONFIG_CRC16_ENABLED.
 *
 * The macro may be defined as:
 * @c CRC16_BASIC: Basic method. No memory tables used. This method is about 8
 *     times slower than the tabled method but no extra memory is used.
 * @c CRC16_TAB_16LH: 2x16 elements table, 2 bytes each.
 */
#define CONFIG_CRC16_ALGO CRC16_TAB_16LH

/**
 * Store CRC tables in flash memory instead of RAM.
 * Valid only if CRC algorithms are configured for memory tables usage.
 *
 * @note The configuration reduces RAM usage (usually most constrained resource
 *     on embedded devices) by the library instead of flash, but also increases
 *     time needed for CRC calculation, since the flash access is much slower
 *     than RAM.
 */
//#define CONFIG_FLASH_CRC_TAB

/**
 * GPIO blink reveals as a short, unexpected low-high (or vice versa) state
 * change on the digital data wire. The GPIO blink (resulted of the internal
 * library logic) may occur if both of the following circumstances occur:
 *
 * 1. The driver is configured for parasitically powering slaves via GPIO
 *    bit-banging (possible for non open-drain type of output).
 * 2. The underlying platform can't guarantee input to output GPIO switch with
 *    a desired initial state.
 *
 * To prevent the blink resulted of the above @c CONFIG_BUS_BLINK_PROTECTION
 * may be configured. Undesirable side effect of using this parameter is a short
 * period of time when a direct voltage source is provided directly on the data
 * wire. This is basically unwanted behavior in the open-drain environment
 * (except parasitically powering slave devices in a specific period of time
 * during 1-wire activity). The side effect occurs while switching the data
 * wire GPIO from low to high state via the following 3-steps procedure:
 *
 *  1. Initial low state (GPIO configured as output-low).
 *  2. Hight state (GPIO configured as output-high) - direct voltage source
 *     connected to the bus. This is an additional state provided by @c
 *     CONFIG_BUS_BLINK_PROTECTION parameter.
 *  3. High state visible via pull-up resistor (GPIO configured as input).
 *
 * @note @c CONFIG_BUS_BLINK_PROTECTION may be useful to improve 1-wire
 *     communication on long cables by speeding-up low-to-high transition.
 */
//#define CONFIG_BUS_BLINK_PROTECTION

/**
 * Enable extended virtual interface.
 *
 * The extended interface enables more advance 1-wire service drivers to be
 * implemented in the future. The penalty is additional overhead needed for
 * calling the virtual methods being part of this interface. Since current
 * implementation doesn't leverage the interface it's recommended to disable
 * this parameter for slower platforms.
 */
//#define CONFIG_EXT_VIRTUAL_INTF

/**
 * For C++11 range-loop used during search-scan process to detect slave devices
 * connected to the bus, the parameter specifies number or the search command
 * resends in case of CRC or bus error. The parameter may be useful for error
 * prone setups.
 *
 * If not defined or 0 - no retires.
 */
#define CONFIG_ITERATION_RETRIES 0

/**
 * In case a toolchain doesn't support C++ <new> header or the support is
 * insufficient, use this parameter to enable alternative implementation
 * of basic C++ allocation functionality as C's malloc(), free() counterparts.
 */
//#define CONFIG_CPP_NEW_ALT

/**
 * Dallas thermometers specific.
 *
 * By default DS18S20 uses 9-bits temperature measurement resolution.
 * Enabling this parameter turns on extended resolution calculation for this
 * type of sensors.
 */
//#define CONFIG_DS18S20_EXT_RES

/**
 * For ESP8266 platform there were reported problems when no extra time
 * has been given after the 1-wire service creation. This parameter specifies
 * amount of time (in msecs) the service delays after performing its
 * initialization but before being ready to use.
 *
 * If not defined or 0 - no delay.
 */
#define CONFIG_ESP8266_INIT_TIME 500

#endif /* !OWNG_DISABLE_DEFAULT_CONFIG */
#endif /* __OWNG_CONFIG__ */

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

/**
 * @file config.h Default library configuration.
 */
#ifndef __OWNG_CONFIG__
#define __OWNG_CONFIG__

#ifdef OWNG_CONFIG_FILE
/* use user defined config file instead of this one */
# include OWNG_CONFIG_FILE
#elif !defined(OWNG_DISABLE_DEFAULT_CONFIG)

/**
 * Fast, table based, CRC-8 calculation. If not defined CRC-8 calculation
 * is about 8 times slower but no extra memory is used for the table.
 *
 * The macro may be defined as:
 * @c CRC8_TAB_256: 256 elements table; fastest; significant memory usage.
 * @c CRC8_TAB_16LH: 2x16 elements table; decent fast & acceptable memory usage.
 */
#define CONFIG_CRC8_TAB CRC8_TAB_16LH

/**
 * Store CRC-8 table in flash memory instead of RAM
 * Valid only if @c CONFIG_CRC8_TAB is defined.
 *
 * @note The configuration reduces RAM usage (usually most constrained resource
 *     on embedded devices) by the library instead of flash, but also increases
 *     time needed for CRC calculation, since the flash access is much slower
 *     than RAM.
 */
//#define CONFIG_FLASH_CRC8_TAB

/**
 * See @sa OneWireNg_BitBang::setGpioAsOutput() for more information
 */
//#define CONFIG_BUS_BLINK_PROTECTION

#endif /* !OWNG_DISABLE_DEFAULT_CONFIG */
#endif /* __OWNG_CONFIG__ */

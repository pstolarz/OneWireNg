/*
 * Copyright (c) 2021,2022 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __OWNG_PLATFORM_DELAY__
#define __OWNG_PLATFORM_DELAY__

#ifdef ARDUINO
# include "Arduino.h"
# include "platform/Platform_TimeCritical.h"

# ifndef CONFIG_BITBANG_DELAY_CCOUNT
#  define delayUs(__us) delayMicroseconds(__us)
# elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#  define F_CPU_MHZ (F_CPU / 1000000)

/*
 * TC_CCNT_ADJST: cycles counter adjustment
 * (important for accurate timings on lower frequencies).
 */
#   if F_CPU_MHZ >= 20
#    define TC_CCNT_ADJST 15
#   else
#    define TC_CCNT_ADJST 0
#   endif

#  ifdef ARDUINO_ARCH_ESP32
/*
 * Delay may be performed in two modes:
 * - Relaxed (aside critical section) with interrupt re-entrancy enabled.
 *   Accuracy is not required; delayMicroseconds() used.
 * - Strict (inside critical section) with interrupt disabled. Timing
 *   accuracy reached by CPU clock cycles tracking.
 */
#   define delayUs(__us) \
    if (_tc[xPortGetCoreID()].actv) { \
        unsigned stop = (_tc[xPortGetCoreID()].ccnt += \
            ((__us) * F_CPU_MHZ) + TC_CCNT_ADJST); \
        while ((int)(stop - get_cpu_cycle_count()) > 0); \
    } else { \
        delayMicroseconds(__us); \
    }
#  else
#   define delayUs(__us) \
    if (_tc_actv) { \
        unsigned stop = (_tc_ccnt += \
            ((__us) * F_CPU_MHZ + TC_CCNT_ADJST)); \
        while ((int)(stop - get_cpu_cycle_count()) > 0); \
    } else { \
        delayMicroseconds(__us); \
    }
#  endif
# else /* ESP32 || ESP8266 */
#  define delayUs(__us) delayMicroseconds(__us)
# endif
# define delayMs(__ms) delay(__ms)
#else /* ARDUINO */
# ifdef __TEST__
#  include <unistd.h>
#  define delayUs(__us) usleep(__us)
#  define delayMs(__ms) usleep(1000L * (__ms))
# else
#  error "Delay API unsupported for the target platform."
# endif
#endif

#endif /* __OWNG_PLATFORM_DELAY__ */

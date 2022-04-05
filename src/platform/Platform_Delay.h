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

#include "platform/Platform_TimeCritical.h"

#ifdef IDF_VER
# include "freertos/task.h"
void idf_delayUs(uint32_t us);
# define delayMs(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)
# define _delayUs(us) idf_delayUs(us)
#elif defined(ARDUINO)
# define delayMs(ms) delay(ms)
# define _delayUs(us) delayMicroseconds(us)
#elif defined(__MBED__)
# ifndef NO_RTOS
#  define delayMs(ms) rtos::ThisThread::sleep_for(ms * 1ms)
# else
#  define delayMs(ms) wait_us(ms * 1000)
# endif
# define _delayUs(us) wait_us(us)
#elif __TEST__
# include <unistd.h>
# define delayMs(ms) usleep(1000L * (ms))
# define delayUs(us) usleep(us)
#else
# error "Delay API unsupported for the target platform."
#endif

#ifdef CONFIG_BITBANG_DELAY_CCOUNT
# if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) || defined(IDF_VER)
#  ifdef F_CPU
#   define F_CPU_MHZ (F_CPU / 1000000)
#  elif CONFIG_IDF_TARGET_ESP32
#   define F_CPU_MHZ CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ
#  elif CONFIG_IDF_TARGET_ESP32S2
#   define F_CPU_MHZ CONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ
#  elif CONFIG_IDF_TARGET_ESP32S3
#   define F_CPU_MHZ CONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ
#  elif CONFIG_IDF_TARGET_ESP32C3
#   define F_CPU_MHZ CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ
#  elif CONFIG_IDF_TARGET_ESP32H2
#   define F_CPU_MHZ CONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ
#  elif CONFIG_IDF_TARGET_ESP8266
#   define F_CPU_MHZ CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ
#  else
#   error "Can't detect CPU frequency while configured with CONFIG_BITBANG_DELAY_CCOUNT"
#  endif

/*
 * TC_CCNT_ADJST: cycles counter adjustment
 * (important for accurate timings on lower frequencies).
 */
#  if F_CPU_MHZ >= 20
#   define TC_CCNT_ADJST 15
#  else
#   define TC_CCNT_ADJST 0
#  endif

/*
 * Delay may be performed in two modes:
 * - Relaxed (aside critical section) with interrupt re-entrancy enabled.
 *   Accuracy is not required.
 * - Strict (inside critical section) with interrupt disabled. Timing
 *   accuracy reached by CPU clock cycles tracking.
 */
#  if defined(ARDUINO_ARCH_ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)
#   define delayUs(us) \
    if (_tc_actv) { \
        unsigned stop = (_tc_ccnt += \
            ((unsigned)(us) * F_CPU_MHZ + TC_CCNT_ADJST)); \
        while ((int)(stop - get_cpu_cycle_count()) > 0); \
    } else { \
        _delayUs(us); \
    }
#  else
#   define delayUs(us) \
    if (_tc[xPortGetCoreID()].actv) { \
        unsigned stop = (_tc[xPortGetCoreID()].ccnt += \
            ((unsigned)(us) * F_CPU_MHZ) + TC_CCNT_ADJST); \
        while ((int)(stop - get_cpu_cycle_count()) > 0); \
    } else { \
        _delayUs(us); \
    }
#  endif
# endif /* ESP */
#endif /* CONFIG_BITBANG_DELAY_CCOUNT */

#ifndef delayUs
# define delayUs(us) _delayUs(us)
#endif

#endif /* __OWNG_PLATFORM_DELAY__ */

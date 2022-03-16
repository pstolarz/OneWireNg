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

#ifndef __OWNG_PLATFORM_TIME_CRITICAL__
#define __OWNG_PLATFORM_TIME_CRITICAL__

#ifdef ARDUINO
# include "Arduino.h"
# ifdef ARDUINO_ARCH_ESP32
#  include "freertos/task.h"
#  ifdef CONFIG_IDF_TARGET_ESP32C3
#   include "hal/cpu_hal.h"
#   define get_cpu_cycle_count() cpu_hal_get_cycle_count()
#  else
#   include "xtensa/hal.h"
#   define get_cpu_cycle_count() xthal_get_ccount()
#  endif

typedef struct {
    unsigned int_lev;   /* saved interrupt level */
    unsigned ccnt;      /* cycle counter (at delay start) */
    bool actv;          /* is time critical section active? */
} esp_tc_t;

extern esp_tc_t esp_tc[portNUM_PROCESSORS];

/*
 * NOTE: Arduino ESP32 has interrupts() and noInterrupts() implemented as NOPs,
 * therefore they don't provide proper timing guards in time critical parts of
 * code.
 *
 * This implementation of time critical enter/exit routines bases on Xtensa
 * FreeRTOS API for disabling/enabling interrupts locally (exclusively for
 * a CPU core the routine is called on).
 *
 * Additionally cycle counter state is saved at the entry point for a purpose
 * of accurate timings calculation.
 */
#  define timeCriticalEnter() \
    esp_tc[xPortGetCoreID()].actv = true; \
    esp_tc[xPortGetCoreID()].int_lev = portSET_INTERRUPT_MASK_FROM_ISR(); \
    esp_tc[xPortGetCoreID()].ccnt = get_cpu_cycle_count()

#  define timeCriticalExit() \
    portCLEAR_INTERRUPT_MASK_FROM_ISR(esp_tc[xPortGetCoreID()].int_lev); \
    esp_tc[xPortGetCoreID()].actv = false

# else
#  define timeCriticalEnter() noInterrupts()
#  define timeCriticalExit() interrupts()
# endif
#else
# ifndef __TEST__
#  warning "Time critical API unsupported for the target platform. Disabled."
# endif
# define timeCriticalEnter()
# define timeCriticalExit()
#endif

/*
 * Time critical routines need to be marked by TIME_CRITICAL attribute.
 * Currently it's required for ESP platforms only to place the routines
 * into IRAM.
 */
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
# define TIME_CRITICAL IRAM_ATTR
#else
# define TIME_CRITICAL
#endif

#endif /* __OWNG_PLATFORM_TIME_CRITICAL__ */

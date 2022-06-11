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

#include "platform/Platform_Delay.h"

#if CONFIG_BITBANG_DELAY_CCOUNT
# if defined(ARDUINO_ARCH_ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)
unsigned cpuFreqMhz = 0;
unsigned ccntAdjst = 0;

#  ifdef ARDUINO
#   include "core_esp8266_features.h"

unsigned ccntUpdateCpuFreqMHz(void)
{
    cpuFreqMhz =  (unsigned)esp_get_cpu_freq_mhz();
    ccntAdjst = 15;
    return cpuFreqMhz;
}
#  else
/* IDF: ESP8266 RTOS SDK */
#   include "driver/rtc.h"

unsigned ccntUpdateCpuFreqMHz(void)
{
    switch (rtc_clk_cpu_freq_get())
    {
    case RTC_CPU_FREQ_80M:
        cpuFreqMhz =  80;
        break;

    case RTC_CPU_FREQ_160M:
        cpuFreqMhz =  160;
        break;
    }
    ccntAdjst = 15;
    return cpuFreqMhz;
}
#  endif
# elif defined(ARDUINO_ARCH_ESP32) || defined(IDF_VER)
#  include "soc/rtc.h"

unsigned cpuFreqMhz = 0;
unsigned ccntAdjst = 0;

unsigned ccntUpdateCpuFreqMHz(void)
{
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);

    cpuFreqMhz =  (unsigned)conf.freq_mhz;
    ccntAdjst = (cpuFreqMhz >= 20 ? 15 : 0);

    return cpuFreqMhz;
}
# endif
#endif /* CONFIG_BITBANG_DELAY_CCOUNT */

#ifdef IDF_VER
# include "esp_timer.h"

TIME_CRITICAL void idf_delayUs(uint32_t us)
{
    if (us > 0) {
        uint64_t stop = ((uint64_t)esp_timer_get_time() + us);
        while((int64_t)(stop - (uint64_t)esp_timer_get_time()) > 0);
    }
}
#endif

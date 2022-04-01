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

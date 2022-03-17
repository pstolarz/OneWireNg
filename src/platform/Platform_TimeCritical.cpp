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

#include "platform/Platform_TimeCritical.h"

#ifdef ARDUINO_ARCH_ESP32
tc_t _tc[portNUM_PROCESSORS];
#elif ARDUINO_ARCH_ESP8266
unsigned _tc_ccnt;
bool _tc_actv;
#endif

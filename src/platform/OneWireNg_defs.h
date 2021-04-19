/*
 * Copyright (c) 2021 Piotr Stolarz
 * OneWireNg: Ardiono 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

/*
 * This header specifies platform specific defines used by the library.
 */

#ifndef __OWNG_PLATFORM_DEFS__
#define __OWNG_PLATFORM_DEFS__

#ifdef ARDUINO
# include "Arduino.h"
# define delayUs(__us) delayMicroseconds(__us)
# define delayMs(__ms) delay(__ms)
# define timeCriticalEnter() noInterrupts()
# define timeCriticalExit() interrupts()
#elif defined(__TEST__)
# include <unistd.h>
# define delayUs(__us) usleep(__us)
# define delayMs(__ms) usleep(1000L * (__ms))
# define timeCriticalEnter()
# define timeCriticalExit()
#else
# error "Unsupported platform"
#endif

#endif /* __OWNG_PLATFORM_DEFS__ */

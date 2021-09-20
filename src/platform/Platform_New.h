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

#ifndef __OWNG_PLATFORM_NEW__
#define __OWNG_PLATFORM_NEW__

#if __cplusplus >= 201103L
# define PTR_ALIGNED alignas(void*)
# define NOEXCEPT noexcept
#else
# define PTR_ALIGNED __attribute__ ((aligned(sizeof(void*))))
# define NOEXCEPT throw()
#endif

/*
 * Some toolchains (e.g. Arduino megaAVR) don't provide standard C++ headers
 * but only implement supported features partially. This header tries to detect
 * if current toolchain provides C++ <new> header and if not overloads
 * new-operator for the in-place variant (as required by the library).
 */
#ifdef ARDUINO
# include "Arduino.h"

# if !(defined(_NEW) || defined(NEW_H))
inline void *operator new(size_t size, void *ptr) NOEXCEPT {
    (void)size;
    return ptr;
}
# endif
#else
# include <new>
#endif

#endif /* __OWNG_PLATFORM_NEW__ */

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

/*
 * Some toolchains (e.g. Arduino megaAVR) don't provide standard C++ headers
 * but only implement supported features partially. This header tries to detect
 * if current toolchain provides C++ <new> header and if not overloads
 * new-operator for the in-place variant (as required by the library).
 */
#ifdef ARDUINO
/* if an Arduino toolchain supports <new> it's usually included by Arduino.h */
# include "Arduino.h"

# ifndef _NEW
inline void *operator new(size_t size, void *ptr)
#  if __cplusplus >= 201103L
    noexcept
#  else
    throw()
#  endif
{
    (void)size;
    return ptr;
}
# endif
#else
# include <new>
#endif

#if __cplusplus >= 201103L
# define PTR_ALIGNED alignas(void*)
#else
# define PTR_ALIGNED __attribute__ ((aligned(sizeof(void*))))
#endif

#endif /* __OWNG_PLATFORM_NEW__ */

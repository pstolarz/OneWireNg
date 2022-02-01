#ifndef __OWNG_TEST_COMMON__
#define __OWNG_TEST_COMMON__

#include <cassert>
#include <cstdio>
#include <cstring>

#include "OneWireNg_BitBang.h"

#define TAB_SZ(t) (sizeof(t)/sizeof(t[0]))

#define TEST_SUCCESS() \
    printf("%s: SUCCESS\n", __FUNCTION__)

#endif /* __OWNG_TEST_COMMON__ */

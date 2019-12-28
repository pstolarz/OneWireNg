/*
 * Copyright (c) 2019 Piotr Stolarz
 * OneWireNg: Ardiono 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include "OneWireNg.h"
#include <string.h>

#define CRC8_TAB_256    1
#define CRC8_TAB_16LH   2

#if defined(CONFIG_CRC8_TAB) && \
    !(CONFIG_CRC8_TAB == CRC8_TAB_256 || CONFIG_CRC8_TAB == CRC8_TAB_16LH)
# error Invalid CONFIG_CRC8_TAB
#endif

/* flash storage API */
#if defined(CONFIG_CRC8_TAB) && defined(CONFIG_FLASH_CRC8_TAB)
# ifdef ARDUINO
#  include "Arduino.h"
#  define CRCTAB_STORAGE PROGMEM
#  define crcTabRead(addr) pgm_read_byte(addr)
# else
#  ifndef __TEST__
#   warning "CONFIG_FLASH_CRC8_TAB unsupported for the target platform"
#  endif
#  define CRCTAB_STORAGE
#  define crcTabRead(addr) ((uint8_t)(*(addr)))
# endif
#else
# define CRCTAB_STORAGE
# define crcTabRead(addr) ((uint8_t)(*(addr)))
#endif

uint8_t OneWireNg::touchByte(uint8_t byte)
{
    uint8_t ret = 0;
    for (int i=0; i < 8; i++) {
        if (touchBit(byte & 1)) ret |= 1 << i;
        byte >>= 1;
    }
    return ret;
}

#define __UPDATE_DISCREPANCY() \
    (memcpy(_lsrch, id, sizeof(Id)), ((_lzero = lzero) < 0))

OneWireNg::ErrorCode OneWireNg::search(Id& id, bool alarm)
{
#if (CONFIG_MAX_SRCH_FILTERS > 0)
restart:
#endif
    int lzero = -1;
    memset(&id, 0, sizeof(Id));

    /* initialize search process on slave devices */
    ErrorCode err = reset();
    if (err != EC_SUCCESS)
        return err;

#if (CONFIG_MAX_SRCH_FILTERS > 0)
    searchFilterSelectAll();
#endif
    touchByte(alarm ? CMD_SEARCH_ROM_COND : CMD_SEARCH_ROM);

    for (int n=0; n < (int)(8*sizeof(Id)); n++)
    {
        ErrorCode ec = transmitSearchTriplet(n, id, lzero);

#if (CONFIG_MAX_SRCH_FILTERS > 0)
        if (ec == EC_FILTERED) {
            if (__UPDATE_DISCREPANCY())
                return EC_NO_DEVS;
            else
                goto restart;
        } else
#endif
        if (ec != EC_SUCCESS)
            return ec;
    }

    err = checkCrcId(id);
    if (err != EC_SUCCESS)
        return err;

    return (__UPDATE_DISCREPANCY() ? EC_DONE : EC_MORE);
}

#undef __UPDATE_DISCREPANCY

#define __BITMASK8(n)       ((uint8_t)(1 << ((n) & 7)))
#define __BYTE_OF_BIT(t, n) ((t)[(n) >> 3])
#define __BIT_IN_BYTE(t, n) (__BYTE_OF_BIT(t, n) & __BITMASK8(n))
#define __BIT_SET(t, n)     (__BYTE_OF_BIT(t, n) |= __BITMASK8(n))

#if (CONFIG_MAX_SRCH_FILTERS > 0)
OneWireNg::ErrorCode OneWireNg::searchFilterAdd(uint8_t code)
{
    for (int i=0; i < _n_fltrs; i++) {
        /* check if the code is already added */
        if (_fltrs[i].code == code)
            return EC_SUCCESS;
    }

    if (_n_fltrs >= CONFIG_MAX_SRCH_FILTERS)
        return EC_FULL;

    _fltrs[_n_fltrs].code = code;
    _fltrs[_n_fltrs].ns = false;
    _n_fltrs++;

    return EC_SUCCESS;
}

void OneWireNg::searchFilterDel(uint8_t code)
{
    for (int i=0; i < _n_fltrs; i++) {
        if (_fltrs[i].code == code) {
            for (i++; i < _n_fltrs; i++) {
                _fltrs[i-1].code = _fltrs[i].code;
            }
            _n_fltrs--;
            break;
        }
    }
}

int OneWireNg::searchFilterApply(int n)
{
    if (!_n_fltrs)
        /* no filtering - any bit value applies */
        return 2;

    uint8_t ba=0, bo=0, bm=__BITMASK8(n);
    ba--; /* all 1s */

    for (int i=0; i < _n_fltrs; i++) {
        if (!_fltrs[i].ns) {
            ba &= _fltrs[i].code;
            bo |= _fltrs[i].code;
        }
    }
    return (!(bo & bm) ? 0 : ((ba & bm) ? 1 : 2));
}

void OneWireNg::searchFilterSelect(int n, int bit)
{
    uint8_t bm=__BITMASK8(n);
    for (int i=0; i < _n_fltrs; i++) {
        if (!_fltrs[i].ns) {
            if ((((_fltrs[i].code & bm) != 0) ^ (bit != 0)))
                _fltrs[i].ns = true;
        }
    }
}
#endif /* CONFIG_MAX_SRCH_FILTERS */

/**
 * Transmit search triplet on the bus (for a given bit position @c n)
 * consisting of the following bits:
 * - bit 1: 0 present for this bit position (master read, slave write),
 * - bit 2: 1 present for this bit position (master read, slave write),
 * - bit 3: select slave with a given bit value (master write, slave read).
 *     This bit may not be transmitted in case it has no sense (no slave
 *     devices on the bus or bus error).
 *
 * If selected bit value is 1 then the corresponding n-th bit in @c id is set
 * (the @id shall be initialized with 0).
 *
 * @c lzero is set to @c n if discrepancy occurred at the processed bit and
 * the bit value is 0. @c lzero is not updated in other case.
 *
 * @return Error codes: @sa EC_SUCCESS, @sa EC_BUS_ERROR, @sa EC_FILTERED.
 */
OneWireNg::ErrorCode OneWireNg::transmitSearchTriplet(int n, Id& id, int& lzero)
{
    int selBit; /* selected bit value */

    int v0 = touchBit(1);   /* 0-presence */
    int v1 = touchBit(1);   /* 1-presence */

    if (v1 && v0)
    {
        /*
         * No slave devices present on the bus. Reset pulse
         * indicated presence of some - bus error is returned.
         */
        return EC_BUS_ERROR;
    } else
    if (!v1 && !v0)
    {
        /*
         * Discrepancy detected for this bit position.
         */
        if (n >= (int)(8*(sizeof(Id)-1))) {
            /* no discrepancy is expected for CRC part of the id - bus error */
            return EC_BUS_ERROR;
        } else {
#if (CONFIG_MAX_SRCH_FILTERS > 0)
            if (n >= 8 || (selBit = searchFilterApply(n)) == 2)
#endif
            {
                if (n < _lzero) {
                    selBit = (__BIT_IN_BYTE(_lsrch, n) != 0);
                } else
                if (n == _lzero) {
                    selBit = 1;
                } else {
                    selBit = 0;
                }

                if (!selBit)
                    lzero = n;
            }
        }
    } else
    {
        /*
         * Unambiguous value for this bit position.
         */
        selBit = !v1;
#if (CONFIG_MAX_SRCH_FILTERS > 0)
        if (n < 8)
        {
            /* check if code matches filtering criteria */
            int fltBit = searchFilterApply(n);
            if (fltBit != 2 && fltBit != selBit)
                return EC_FILTERED;
        }
#endif
    }

    touchBit(selBit);
#if (CONFIG_MAX_SRCH_FILTERS > 0)
    searchFilterSelect(n, selBit);
#endif
    if (selBit) {
        __BIT_SET(id, n);
    }
    return EC_SUCCESS;
}

#undef __BIT_SET
#undef __BIT_IN_BYTE
#undef __BYTE_OF_BIT
#undef __BITMASK8

uint8_t OneWireNg::crc8(const void *in, size_t len)
{
    uint8_t crc = 0;
    const uint8_t *in_bts = (const uint8_t*)in;

#if (CONFIG_CRC8_TAB == CRC8_TAB_256)
    static const uint8_t CRCTAB_STORAGE CRC8_256[] =
    {
        0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
        0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
        0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
        0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
        0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
        0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
        0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
        0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
        0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
        0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
        0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
        0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
        0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
        0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
        0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
        0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
        0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
        0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
        0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
        0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
        0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
        0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
        0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
        0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
        0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
        0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
        0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
        0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
        0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
        0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
        0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
        0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
    };

    while (len--) {
        crc = crc ^ *in_bts++;
        crc = crcTabRead(CRC8_256 + (crc & 0xff));
    }
#elif (CONFIG_CRC8_TAB == CRC8_TAB_16LH)
    static const uint8_t CRCTAB_STORAGE CRC8_16L[] = {
        0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
        0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41
    };
    static const uint8_t CRCTAB_STORAGE CRC8_16H[] = {
        0x00, 0x9d, 0x23, 0xbe, 0x46, 0xdb, 0x65, 0xf8,
        0x8c, 0x11, 0xaf, 0x32, 0xca, 0x57, 0xe9, 0x74
    };

    while (len--) {
        crc = crc ^ *in_bts++;
        crc = crcTabRead(CRC8_16L + (crc & 0x0f)) ^
            crcTabRead(CRC8_16H + ((crc >> 4) & 0x0f));
    }
#else
    int i;
    uint8_t b;

    while (len--) {
        b = *in_bts++;
        for (i=8; i; i--, b >>= 1)
            crc = ((crc ^ b) & 1 ? 0x8c : 0) ^ (crc >> 1);
    }
#endif

    return crc;
}

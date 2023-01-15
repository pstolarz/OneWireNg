/*
 * Copyright (c) 2019-2023 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include "common.h"

#define MAX_TEST_SLAVES 20

// static const OneWireNg::Id ZERO_ID = {};
static const OneWireNg::Id TEST1_IDS[] =
{
    {0x28, 0xff, 0x11, 0x8a, 0x60, 0x14, 0x02, 0xf5},
    {0x28, 0xff, 0x3c, 0x6e, 0x2d, 0x04, 0x00, 0xd7},
    {0x28, 0xff, 0x87, 0x34, 0x2e, 0x04, 0x00, 0xcf},
    {0x28, 0x05, 0x37, 0x3c, 0x05, 0x00, 0x00, 0x4f},
    {0x28, 0xff, 0x54, 0x88, 0x2c, 0x04, 0x00, 0x13},
    {0x28, 0xff, 0x9e, 0x34, 0x2e, 0x04, 0x00, 0x40},
    {0x28, 0x87, 0xf4, 0xef, 0x04, 0x00, 0x00, 0x85},
    {0x28, 0xff, 0x2b, 0x45, 0x4c, 0x04, 0x00, 0x10},
    {0x28, 0xc8, 0xeb, 0xa0, 0x04, 0x00, 0x00, 0xbd},
    {0x28, 0x27, 0x9b, 0xa1, 0x04, 0x00, 0x00, 0x52},
    {0x28, 0xe3, 0xd7, 0xa1, 0x04, 0x00, 0x00, 0xd9},
    {0x28, 0x3f, 0x1c, 0x31, 0x02, 0x00, 0x00, 0x02},
    {0x28, 0x1d, 0x39, 0x31, 0x02, 0x00, 0x00, 0xf0},
    {0x28, 0xb1, 0x6d, 0xa1, 0x03, 0x00, 0x00, 0x11},
    {0x28, 0x87, 0x6a, 0xa1, 0x03, 0x00, 0x00, 0x1f},
    {0x28, 0x48, 0x60, 0xbf, 0x06, 0x00, 0x00, 0x01},
    {0x28, 0x9a, 0x8f, 0x24, 0x06, 0x00, 0x00, 0x6b}
};
static const OneWireNg::Id TEST2_IDS[] =
{
    {0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x74},
    {0x44, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xb3},
    {0x44, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0xf1},
//    {0xc4, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x24},
    {0xb4, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xf7},
    {0x74, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xa7},
    {0x7c, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x56},
    {0xfc, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xc1},
//    {0x47, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xf4},
    {0xc7, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x63},
    {0x77, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xe0},
    {0xf7, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x77},
    {0xf7, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x35},
    {0x4f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x05},
    {0x2f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x2d},
    {0x2f, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x6f},
//    {0xdf, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x69},
    {0x7f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x11},
    {0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x86}
};
static const uint8_t TEST2_FILTERS[] = {
    0xc4, 0x74, 0x7c, 0x47, 0xf7, 0x2f, 0xdf
};

class OneWireNg_Test: OneWireNg
{
private:
    OneWireNg_Test() {
        _slaves_n = 0;
    }

    int searchHandler(int bit)
    {
        bit = (bit != 0);   /* 0/1 conversion */
        int resAnd = bit;

        int bit_n = (_trans_n-8) / 3;
        int trpl_n = (_trans_n-8) % 3;

        for (int i = 0; i < _slaves_n; i++)
        {
            if (!_slaves[i].srchIdle)
            {
                int bv = (_slaves[i].id[bit_n >> 3] & (1 << (bit_n & 7))) != 0;

                if (trpl_n == 2) {
                    /* select bit */
                    if ((bv ^ bit) != 0)
                        _slaves[i].srchIdle = true;
                } else {
                    /* presence bit */
                    resAnd = resAnd && (!trpl_n ? bv : !bv);
                }
            }
        }
        _trans_n++;
        return resAnd;
    }

    void addSlave(const Id& id)
    {
        if (_slaves_n < MAX_TEST_SLAVES) {
            memcpy(&_slaves[_slaves_n].id, &id, sizeof(Id));
            _slaves[_slaves_n].srchIdle = false;
            _slaves_n++;
        }
    }

    void delAllslaves() {
        _slaves_n = 0;
    }

    static bool cmpId(const Id& id1, const Id& id2)
    {
        return !memcmp(&id1, &id2, sizeof(Id));
    }

    /** Print @c id in BE order */
    static void printId(const Id& id)
    {
        printf("%02x", id[0]);
        for (int i = 1; i < (int)sizeof(Id); i++)
            printf(":%02x", id[i]);
        printf("\n");
    }

    int _trans_n;   /* number of transmitted bits after reset */
    uint8_t _cmd;   /* command id */

    /* emulated slave devices connected to the bus */
    struct {
        Id id;
        bool srchIdle;
    } _slaves[MAX_TEST_SLAVES];
    int _slaves_n;

public:
    ErrorCode reset()
    {
        _trans_n = 0;
        _cmd = 0x00;

        for (int i = 0; i < _slaves_n; i++)
            _slaves[i].srchIdle = false;

        return (_slaves_n > 0 ? EC_SUCCESS : EC_NO_DEVS);
    }

    int touchBit(int bit, bool power)
    {
        (void)power;

        if (_trans_n < 8)
        {
            /* read until completed command id */
            if (bit) _cmd |= (1 << _trans_n);
            _trans_n++;
            return (bit != 0);
        }

        switch (_cmd)
        {
        case CMD_SEARCH_ROM:
        case CMD_SEARCH_ROM_COND:
            return searchHandler(bit);

        default:
            _trans_n++;
            return (bit != 0);
        }
    }

    static void test_crc8()
    {
        for (size_t i = 0; i < TAB_SZ(TEST1_IDS); i++)
            assert(checkCrcId(TEST1_IDS[i]) == EC_SUCCESS);

        TEST_SUCCESS();
    }

    static void test_crc16()
    {
        uint16_t c1 = 0, c2;
        const uint16_t res[] = {
            0xbad3, 0x4631, 0x47bb, 0x3840, 0x477e, 0xa7bc, 0x78e1, 0xd7dd,
            0xb74e, 0x8043, 0x2706, 0x3bc4, 0xd4a5, 0x35a5, 0x0314, 0x4525
        };

        uint8_t buf[0x1000];
        for (size_t i = 0; i < sizeof(buf); i++)
            buf[i] = (uint8_t)i;

        for (size_t i = 0, j = 0; i < sizeof(buf); i += 0x100, j++) {
            c1 = crc16(&buf[i], 0x100, c1);
            c2 = crc16(&buf[0], i + 0x100);
            assert(c1 == c2 && c1 == res[j]);
        }

        TEST_SUCCESS();
    }

    static void test_checkInvCrc16()
    {
        const uint16_t res[] = {
            0xbad3, 0x4631, 0x47bb, 0x3840, 0x477e, 0xa7bc, 0x78e1, 0xd7dd,
            0xb74e, 0x8043, 0x2706, 0x3bc4, 0xd4a5, 0x35a5, 0x0314, 0x4525
        };

        uint8_t buf[0x1000];
        for (size_t i = 0; i < sizeof(buf); i++)
            buf[i] = (uint8_t)i;

        for (size_t i = 0, j = 0; i < sizeof(buf); i += 0x100, j++) {
            assert(checkInvCrc16(&buf[0], i + 0x100, ~res[j]) == EC_SUCCESS);
        }

        TEST_SUCCESS();
    }

    static void test_getLSB()
    {
        uint8_t u16[] = {0x02, 0x01};
        uint8_t u32[] = {0x04, 0x03, 0x02, 0x01};

        assert(getLSB_u16(u16) == 0x0102);
        assert(getLSB_u32(u32) == 0x01020304);

        TEST_SUCCESS();
    }

    static void test_search()
    {
        Id id;
        OneWireNg_Test ow;

        /* no devices */
        assert(ow.search(id) == EC_NO_DEVS);

        /* single device; no discrepancies */
        ow.searchReset();
        ow.addSlave(TEST1_IDS[0]);
        assert(ow.search(id) == EC_MORE && cmpId(id, TEST1_IDS[0]));
        assert(ow.search(id) == EC_NO_DEVS);

        /*
         * multiple devices on the bus; discrepancies
         */
        size_t i;
        int fnd[TAB_SZ(TEST1_IDS)] = {};

        ow.searchReset();
        for (i = 1; i < TAB_SZ(TEST1_IDS); i++)
            ow.addSlave(TEST1_IDS[i]);

        while (ow.search(id) == EC_MORE) {
            // printId(id);
            for (i = 0; i < TAB_SZ(TEST1_IDS); i++) {
                if (cmpId(id, TEST1_IDS[i])) {
                    fnd[i]++;
                    break;
                }
            }
        }

        for (i = 0; i < TAB_SZ(TEST1_IDS); i++) {
            /* each id returned only once */
            assert(fnd[i] == 1);
        }

        /* CRC error */
        Id idCorrupt;
        memcpy(&idCorrupt, &TEST1_IDS[0], sizeof(Id));
        idCorrupt[7] = 0x00;

        ow.searchReset();
        ow.delAllslaves();
        ow.addSlave(idCorrupt);
        assert(ow.search(id) == EC_CRC_ERROR);

        TEST_SUCCESS();
    }

    static void test_filter()
    {
        OneWireNg_Test ow;
        assert(!ow._n_fltrs);

        int i;
        for (i = 0; i < CONFIG_MAX_SEARCH_FILTERS; i++) {
            assert(ow.searchFilterAdd(i+1) == EC_SUCCESS);
            assert(!ow._fltrs[i].ns);
        }
        assert(ow._n_fltrs == CONFIG_MAX_SEARCH_FILTERS);

        /* already exist */
        assert(ow.searchFilterAdd(1) == EC_SUCCESS);
        assert(ow._n_fltrs == CONFIG_MAX_SEARCH_FILTERS);

        assert(ow.searchFilterAdd(0) == EC_FULL);
        assert(ow._n_fltrs == CONFIG_MAX_SEARCH_FILTERS);

        ow.searchFilterDel(0);
        assert(ow._n_fltrs == CONFIG_MAX_SEARCH_FILTERS);

        ow.searchFilterDel(1);
        assert(ow._n_fltrs == CONFIG_MAX_SEARCH_FILTERS-1 &&
            ow._fltrs[0].code == 2);

        ow.searchFilterDelAll();
        assert(!ow._n_fltrs);

        assert(ow.searchFilterApply(1 << 0) == 2);

        ow.searchFilterAdd(0x00);
        ow.searchFilterAdd(0x0f);
        ow.searchFilterAdd(0xf0);
        ow.searchFilterAdd(0xaa);
        ow.searchFilterAdd(0xff);

        ow.searchFilterSelectAll();
        assert(ow.searchFilterApply(1 << 3) == 2);

        ow.searchFilterSelect((1 << 3), 1);
        assert(ow._fltrs[0].ns &&
           !ow._fltrs[1].ns &&
            ow._fltrs[2].ns &&
           !ow._fltrs[3].ns &&
           !ow._fltrs[4].ns);

        assert(ow.searchFilterApply(1 << 3) == 1);

        ow.searchFilterSelectAll();
        for (i = 0; i < ow._n_fltrs; i++)
            assert(!ow._fltrs[1].ns);

        ow.searchFilterSelect((1 << 3), 0);
        assert(!ow._fltrs[0].ns &&
           ow._fltrs[1].ns &&
           !ow._fltrs[2].ns &&
           ow._fltrs[3].ns &&
           ow._fltrs[4].ns);

        assert(!ow.searchFilterApply(1 << 3));

        ow.searchFilterDelAll();
        assert(ow.searchFilterApply(1 << 0) == 2);

        ow.searchFilterAdd(0xaa);
        ow.searchFilterSelectAll();
        for (i = 0; i < 8; i++)
            assert(ow.searchFilterApply(1 << i) == (i % 2));

        TEST_SUCCESS();
    }

    static void test_filteredSearch()
    {
        Id id;
        OneWireNg_Test ow;

        /* no matching devices */
        ow.addSlave(TEST2_IDS[0]);

        ow.searchFilterAdd(0x00);
        assert(ow.search(id) == EC_NO_DEVS);

        /* single device matches */
        ow.searchReset();
        ow.searchFilterDelAll();
        ow.searchFilterAdd(0x04);
        assert(ow.search(id) == EC_MORE && cmpId(id, TEST2_IDS[0]));
        assert(ow.search(id) == EC_NO_DEVS);

        /*
         * multiple devices; multiple filters
         */
        size_t i, j;
        int fnd[TAB_SZ(TEST2_IDS)] = {};

        ow.searchReset();
        ow.searchFilterDelAll();

        for (i = 1; i < TAB_SZ(TEST2_IDS); i++)
            ow.addSlave(TEST2_IDS[i]);

        for (i = 0; i < TAB_SZ(TEST2_FILTERS); i++)
            ow.searchFilterAdd(TEST2_FILTERS[i]);

        while (ow.search(id) == EC_MORE) {
            // printId(id);
            for (i = 0; i < TAB_SZ(TEST2_IDS); i++) {
                if (cmpId(id, TEST2_IDS[i])) {
                    fnd[i]++;
                    break;
                }
            }
        }

        for (i = 0; i < TAB_SZ(TEST2_FILTERS); i++) {
            for (j = 0; j < TAB_SZ(TEST2_IDS); j++) {
                if (TEST2_IDS[j][0] == TEST2_FILTERS[i])
                {
                    /* each filtered id returned only once */
                    assert(fnd[j] == 1);
                    /* mark as checked */
                    fnd[j] = -1;
                }
            }
        }

        for (i = 0; i < TAB_SZ(TEST2_IDS); i++) {
            /* slaves with family codes outside
               the filters set are not returned */
            assert(!fnd[i] || fnd[i] == -1);
        }

        TEST_SUCCESS();
    }
};

int main(void)
{
    OneWireNg_Test::test_crc8();
    OneWireNg_Test::test_crc16();
    OneWireNg_Test::test_checkInvCrc16();
    OneWireNg_Test::test_getLSB();
    OneWireNg_Test::test_search();
    OneWireNg_Test::test_filter();
    OneWireNg_Test::test_filteredSearch();

    return 0;
}

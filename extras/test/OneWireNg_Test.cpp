#include "common.h"

#define MAX_TEST_SLAVES 20

static const OneWireNg::Id ZERO_ID = {};
static const OneWireNg::Id TEST_IDS[] =
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
    {0x28, 0x48, 0x60, 0xBF, 0x06, 0x00, 0x00, 0x01},
    {0x28, 0x9A, 0x8F, 0x24, 0x06, 0x00, 0x00, 0x6B}

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

        size_t bit_n = (_trans_n-8) / 3;
        size_t trpl_n = (_trans_n-8) % 3;

        for (size_t i=0; i < _slaves_n; i++)
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
        for (size_t i=1; i < sizeof(Id); i++)
            printf(":%02x", id[i]);
        printf("\n");
    }

    size_t _trans_n;    /* number of transmitted bits after reset */
    uint8_t _cmd;       /* command id */

    /* emulated slave devices connected to the bus */
    struct {
        Id id;
        bool srchIdle;
    } _slaves[MAX_TEST_SLAVES];
    size_t _slaves_n;

public:
    virtual bool reset()
    {
        _trans_n = 0;
        _cmd = 0x00;

        for (size_t i=0; i < _slaves_n; i++)
            _slaves[i].srchIdle = false;

        return true;
    }

    virtual int touchBit(int bit)
    {
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
        for (size_t i=0; i < TAB_SZ(TEST_IDS); i++)
            assert(checkCrcId(TEST_IDS[i]));

        TEST_SUCCESS();
    }

    static void test_updateDiscrepancy()
    {
        OneWireNg_Test ow;

        assert(cmpId(ow._msk, ZERO_ID));
        assert(cmpId(ow._dscr, ZERO_ID));

        memset(ow._msk, 0xff, sizeof(Id));
        memset(ow._dscr, 0xff, sizeof(Id));
        assert(ow.updateDiscrepancy());

        for (int n=55; n >=0 ; n--)
        {
            ow._dscr[n >> 3] &= ~(1 << (n & 7));
            assert(!ow.updateDiscrepancy());
            assert(ow._msk[7] == 0xff);
            assert(ow._dscr[7] == 0xff);

            for (int i=0; i<55; i++) {
                int msk_bit = ow._msk[i >> 3] & (1 << (i & 7));
                int dscr_bit = ow._dscr[i >> 3] & (1 << (i & 7));
                assert((i <= n) ? (msk_bit != 0) : !msk_bit);
                assert((i <= n) ? (dscr_bit != 0) : !dscr_bit);
            }
            memset(ow._msk, 0xff, sizeof(Id));
            memset(ow._dscr, 0xff, sizeof(Id));
        }

        TEST_SUCCESS();
    }

    static void test_search()
    {
        Id id;
        ErrorCode ec;
        OneWireNg_Test ow;
        bool fnd[TAB_SZ(TEST_IDS)] = {};

        /* no devices */
        ec = ow.search(id);
        assert(ec == EC_NO_DEVS);

        /* single device; no discrepancies */
        ow.searchReset();
        ow.addSlave(TEST_IDS[0]);
        ec = ow.search(id);
        assert(ec == EC_DONE);
        assert(cmpId(id, TEST_IDS[0]));

        /* multiple devices on the bus; discrepancies */
        size_t i, j, fnd_n;

        ow.searchReset();
        for (i=1; i < TAB_SZ(TEST_IDS); i++)
            ow.addSlave(TEST_IDS[i]);

        for (i=fnd_n=0; i<TAB_SZ(TEST_IDS); i++)
        {
            ec = ow.search(id);
            assert(ec == (i+1 < TAB_SZ(TEST_IDS) ? EC_MORE : EC_DONE));
            for (j=0; j < TAB_SZ(TEST_IDS); j++) {
                if (cmpId(id, TEST_IDS[j]))
                {
                    assert(!fnd[j]);
                    fnd[j] = true;
                    fnd_n++;
                    break;
                }
            }
        }
        assert(fnd_n == TAB_SZ(TEST_IDS));

        /* CRC error */
        Id idCorrupt;
        memcpy(&idCorrupt, &TEST_IDS[0], sizeof(Id));
        idCorrupt[7] = 0x00;

        ow.searchReset();
        ow.delAllslaves();
        ow.addSlave(idCorrupt);
        ec = ow.search(id);
        assert(ec == EC_CRC_ERROR);

        TEST_SUCCESS();
    }
};

int main(void)
{
    OneWireNg_Test::test_crc8();
    OneWireNg_Test::test_updateDiscrepancy();
    OneWireNg_Test::test_search();

    return 0;
}

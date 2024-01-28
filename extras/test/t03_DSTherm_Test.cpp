/*
 * Copyright (c) 2021,2022,2024 Piotr Stolarz
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
#include "drivers/DSTherm.h"

class DSTherm_Test: OneWireNg
{
public:
    static void test_filterSupportedSlaves()
    {
        DSTherm_Test ow;
        DSTherm dsth(ow);

        /* fill the whole set with supported therms */
        assert(dsth.filterSupportedSlaves() == OneWireNg::EC_SUCCESS &&
            ow.searchFilterSize() == 5);

        ow.searchFilterDelAll();

        /* same as above but with some supported therms preconfigured */
        ow.searchFilterAdd(DSTherm::DS18B20);
        ow.searchFilterAdd(DSTherm::DS18S20);
        assert(dsth.filterSupportedSlaves() == OneWireNg::EC_SUCCESS &&
            ow.searchFilterSize() == 5);

        ow.searchFilterDelAll();

        /* no space; preconfigured with some no therm device */
        ow.searchFilterAdd(0x2d);   // DS2431
        assert(dsth.filterSupportedSlaves() == OneWireNg::EC_FULL &&
            ow.searchFilterSize() == 1);

        ow.searchFilterDelAll();

        /* no space; preconfigured with therm and no therm devices */
        ow.searchFilterAdd(0x2d);   // DS2431
        ow.searchFilterAdd(DSTherm::DS18B20);
        assert(dsth.filterSupportedSlaves() == OneWireNg::EC_FULL &&
            ow.searchFilterSize() == 2);

        ow.searchFilterDelAll();

        TEST_SUCCESS();
    }

    static void test_conversionTime()
    {
        assert(DSTherm::getConversionTime(
            DSTherm::RES_12_BIT) == DSTherm::MAX_CONV_TIME);
        assert(DSTherm::getConversionTime(DSTherm::RES_11_BIT) == 375);
        assert(DSTherm::getConversionTime(DSTherm::RES_10_BIT) == 188);
        assert(DSTherm::getConversionTime(DSTherm::RES_9_BIT) == 94);

        TEST_SUCCESS();
    }

    static void test_scratchpadTemp()
    {
        DSTherm_Test ow;
        OneWireNg::Id id = {};
        uint8_t scrpd_raw[DSTherm::Scratchpad::LENGTH] = {};
        DSTherm::Scratchpad scrpd = DSTherm::Scratchpad(ow, id, scrpd_raw);

        /* values taken from sensors data sheets */

        /* DS18B20 (12-bits resolution) */
        scrpd._id[0] = DSTherm::DS18B20;
        scrpd.setResolution(DSTherm::RES_12_BIT);

        scrpd._scrpd[0] = 0xd0; scrpd._scrpd[1] = 0x07;
        assert(scrpd.getTemp() == 125000 && scrpd.getTemp2() == 2000);
        scrpd._scrpd[0] = 0x50; scrpd._scrpd[1] = 0x05;
        assert(scrpd.getTemp() == 85000 && scrpd.getTemp2() == 1360);
        scrpd._scrpd[0] = 0x91; scrpd._scrpd[1] = 0x01;
        assert(scrpd.getTemp() == 25062 && scrpd.getTemp2() == 401);
        scrpd._scrpd[0] = 0xa2; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 10125 && scrpd.getTemp2() == 162);
        scrpd._scrpd[0] = 0x08; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 500 && scrpd.getTemp2() == 8);
        scrpd._scrpd[0] = 0x00; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 0 && scrpd.getTemp2() == 0);
        scrpd._scrpd[0] = 0xf8; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -500 && scrpd.getTemp2() == -8);
        scrpd._scrpd[0] = 0x5e; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -10125 && scrpd.getTemp2() == -162);
        scrpd._scrpd[0] = 0x6f; scrpd._scrpd[1] = 0xfe;
        assert(scrpd.getTemp() == -25062 && scrpd.getTemp2() == -401);
        scrpd._scrpd[0] = 0x90; scrpd._scrpd[1] = 0xfc;
        assert(scrpd.getTemp() == -55000 && scrpd.getTemp2() == -880);

        /* as above but with 9-bits resolution */
        scrpd.setResolution(DSTherm::RES_9_BIT);

        scrpd._scrpd[0] = 0xd0; scrpd._scrpd[1] = 0x07;
        assert(scrpd.getTemp() == 125000 && scrpd.getTemp2() == 2000);
        scrpd._scrpd[0] = 0x50; scrpd._scrpd[1] = 0x05;
        assert(scrpd.getTemp() == 85000 && scrpd.getTemp2() == 1360);
        scrpd._scrpd[0] = 0x91; scrpd._scrpd[1] = 0x01;
        assert(scrpd.getTemp() == 25000 && scrpd.getTemp2() == 400);
        scrpd._scrpd[0] = 0xa2; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 10000 && scrpd.getTemp2() == 160);
        scrpd._scrpd[0] = 0x08; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 500 && scrpd.getTemp2() == 8);
        scrpd._scrpd[0] = 0x00; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 0 && scrpd.getTemp2() == 0);
        scrpd._scrpd[0] = 0xf8; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -500 && scrpd.getTemp2() == -8);
        scrpd._scrpd[0] = 0x5e; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -10500 && scrpd.getTemp2() == -168);
        scrpd._scrpd[0] = 0x6f; scrpd._scrpd[1] = 0xfe;
        assert(scrpd.getTemp() == -25500 && scrpd.getTemp2() == -408);
        scrpd._scrpd[0] = 0x90; scrpd._scrpd[1] = 0xfc;
        assert(scrpd.getTemp() == -55000 && scrpd.getTemp2() == -880);

        /* DS18S20 (const 9-bits resolution) */
        scrpd._id[0] = DSTherm::DS18S20;
        scrpd._scrpd[7] = 0x10;
        scrpd._scrpd[6] = 0x0c;

        scrpd._scrpd[0] = 0xaa; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 85000 && scrpd.getTemp2() == 1360);
        scrpd._scrpd[0] = 0x32; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 25000 && scrpd.getTemp2() == 400);
        scrpd._scrpd[0] = 0x01; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 500 && scrpd.getTemp2() == 8);
        scrpd._scrpd[0] = 0x00; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 0 && scrpd.getTemp2() == 0);
        scrpd._scrpd[0] = 0xff; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -500 && scrpd.getTemp2() == -8);
        scrpd._scrpd[0] = 0xce; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -25000 && scrpd.getTemp2() == -400);
        scrpd._scrpd[0] = 0x92; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -55000 && scrpd.getTemp2() == -880);

        TEST_SUCCESS();
    }

    static void test_scratchpadConfig()
    {
        DSTherm_Test ow;
        OneWireNg::Id id = {};
        uint8_t scrpd_raw[DSTherm::Scratchpad::LENGTH] = {};
        DSTherm::Scratchpad scrpd = DSTherm::Scratchpad(ow, id, scrpd_raw);

        /* DS1825 */
        scrpd._id[0] = DSTherm::DS1825;

        scrpd.setThl(50, -50);
        scrpd.setResolution(DSTherm::RES_12_BIT);
        scrpd.setAddr(15);
        assert(scrpd.getTh() == 50 && scrpd.getTl() == -50 &&
            scrpd.getResolution() == DSTherm::RES_12_BIT &&
            scrpd.getAddr() == 15);

        scrpd.setThl(100, -100);
        scrpd.setResolution(DSTherm::RES_9_BIT);
        scrpd.setAddr(0);
        assert(scrpd.getTh() == 100 && scrpd.getTl() == -100 &&
            scrpd.getResolution() == DSTherm::RES_9_BIT &&
            scrpd.getAddr() == 0);

        scrpd.setAddr(15);

        /* DS18B20 */
        scrpd._id[0] = DSTherm::DS18B20;

        /* address is not allowed to change */
        scrpd.setThl(1, -1);
        scrpd.setResolution(DSTherm::RES_11_BIT);
        scrpd.setAddr(0);
        assert(scrpd.getTh() == 1 && scrpd.getTl() == -1 &&
            scrpd.getResolution() == DSTherm::RES_11_BIT &&
            scrpd.getAddr() == 15);

        scrpd.setResolution(DSTherm::RES_12_BIT);

        /* DS18S20 */
        scrpd._id[0] = DSTherm::DS18S20;

        /* address and resolution is not allowed to change */
        scrpd.setThl(10, -10);
        scrpd.setResolution(DSTherm::RES_10_BIT);
        scrpd.setAddr(0);
        assert(scrpd.getTh() == 10 && scrpd.getTl() == -10 &&
            scrpd.getResolution() == DSTherm::RES_9_BIT &&
            scrpd.getAddr() == 15);

        TEST_SUCCESS();
    }

private:
    DSTherm_Test() {}

    ErrorCode reset() {
        return OneWireNg::EC_SUCCESS;
    }

    int touchBit(int bit, bool power) {
        (void)power;
        return bit;
    }
};

int main(void)
{
    DSTherm_Test::test_filterSupportedSlaves();
    DSTherm_Test::test_conversionTime();
    DSTherm_Test::test_scratchpadTemp();
    DSTherm_Test::test_scratchpadConfig();

    return 0;
}

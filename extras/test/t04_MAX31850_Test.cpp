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
#include "drivers/MAX31850.h"

class MAX31850_Test: OneWireNg
{
public:
    static void test_scratchpadTemp()
    {
        MAX31850_Test ow;
        OneWireNg::Id id = {};
        uint8_t scrpd_raw[MAX31850::Scratchpad::LENGTH] = {};
        MAX31850::Scratchpad scrpd = MAX31850::Scratchpad(ow, id, scrpd_raw);

        /* values taken from sensors data sheet */

        scrpd._scrpd[0] = 0x00; scrpd._scrpd[1] = 0x64;
        assert(scrpd.getTemp() == 1600000 && scrpd.getTemp2() == 25600 &&
            !scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0x81; scrpd._scrpd[1] = 0x3e;
        assert(scrpd.getTemp() == 1000000 && scrpd.getTemp2() == 16000 &&
            scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0x4e; scrpd._scrpd[1] = 0x06;
        assert(scrpd.getTemp() == 100750 && scrpd.getTemp2() == 1612 &&
            !scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0x93; scrpd._scrpd[1] = 0x01;
        assert(scrpd.getTemp() == 25000 && scrpd.getTemp2() == 400 &&
            scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0x02; scrpd._scrpd[1] = 0x00;
        assert(scrpd.getTemp() == 0 && scrpd.getTemp2() == 0 &&
            !scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0xfc; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -250 && scrpd.getTemp2() == -4 &&
            !scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0xf1; scrpd._scrpd[1] = 0xff;
        assert(scrpd.getTemp() == -1000 && scrpd.getTemp2() == -16 &&
            scrpd.getFaultStatus());
        scrpd._scrpd[0] = 0x63; scrpd._scrpd[1] = 0xf0;
        assert(scrpd.getTemp() == -250000 && scrpd.getTemp2() == -4000 &&
            scrpd.getFaultStatus());

        assert(!scrpd.getAddr());
        for (size_t i = 0; i < sizeof(OneWireNg::Id); i++)
            assert(!scrpd.getId()[i]);

        TEST_SUCCESS();
    }

    static void test_scratchpadTempInternal()
    {
        MAX31850_Test ow;
        OneWireNg::Id id = {};
        uint8_t scrpd_raw[MAX31850::Scratchpad::LENGTH] = {};
        MAX31850::Scratchpad scrpd = MAX31850::Scratchpad(ow, id, scrpd_raw);

        /* values taken from sensors data sheet */

        scrpd._scrpd[3] = 0x7f; scrpd._scrpd[2] = 0x00;
        assert(scrpd.getTempInternal() == 127000 &&
            scrpd.getTempInternal2() == 2032 &&
            !scrpd.getInputState());
        scrpd._scrpd[3] = 0x64; scrpd._scrpd[2] = 0x91;
        assert(scrpd.getTempInternal() == 100562 &&
            scrpd.getTempInternal2() == 1609 &&
            scrpd.getInputState() == MAX31850::INPUT_OC);
        scrpd._scrpd[3] = 0x19; scrpd._scrpd[2] = 0x02;
        assert(scrpd.getTempInternal() == 25000 &&
            scrpd.getTempInternal2() == 400 &&
            scrpd.getInputState() == MAX31850::INPUT_SCG);
        scrpd._scrpd[3] = 0x00; scrpd._scrpd[2] = 0x04;
        assert(scrpd.getTempInternal() == 0 &&
            scrpd.getTempInternal2() == 0 &&
            scrpd.getInputState() == MAX31850::INPUT_SCV);
        scrpd._scrpd[3] = 0xff; scrpd._scrpd[2] = 0xff;
        assert(scrpd.getTempInternal() == -62 &&
            scrpd.getTempInternal2() == -1 &&
            scrpd.getInputState() ==
                (MAX31850::INPUT_OC | MAX31850::INPUT_SCG | MAX31850::INPUT_SCV));
        scrpd._scrpd[3] = 0xff; scrpd._scrpd[2] = 0x00;
        assert(scrpd.getTempInternal() == -1000 &&
            scrpd.getTempInternal2() == -16 &&
            !scrpd.getInputState());
        scrpd._scrpd[3] = 0xec; scrpd._scrpd[2] = 0x06;
        assert(scrpd.getTempInternal() == -20000 &&
            scrpd.getTempInternal2() == -320 &&
            scrpd.getInputState() == (MAX31850::INPUT_SCG | MAX31850::INPUT_SCV));
        scrpd._scrpd[3] = 0xc9; scrpd._scrpd[2] = 0x03;
        assert(scrpd.getTempInternal() == -55000 &&
            scrpd.getTempInternal2() == -880 &&
            scrpd.getInputState() == (MAX31850::INPUT_OC | MAX31850::INPUT_SCG));

        assert(!scrpd.getAddr());
        for (size_t i = 0; i < sizeof(OneWireNg::Id); i++)
            assert(!scrpd.getId()[i]);

        TEST_SUCCESS();
    }

private:
    MAX31850_Test() {}

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
    MAX31850_Test::test_scratchpadTemp();
    MAX31850_Test::test_scratchpadTempInternal();

    return 0;
}

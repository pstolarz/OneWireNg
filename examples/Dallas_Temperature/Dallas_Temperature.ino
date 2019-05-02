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

#include "OneWireNg_CurrentPlatform.h"

#define OW_PIN          10

/* uncomment for parasite powering support */
// #define PARASITE_POWER

#ifdef PARASITE_POWER
/* uncomment for power provided by a switching
   transistor and controlled by this pin */
// # define PWR_CTRL_PIN   9
#endif

#define CONVERT_T           0x44
#define COPY_SCRATCHPAD     0x48
#define WRITE_SCRATCHPAD    0x4e
#define RECALL_EEPROM       0xb8
#define READ_POW_SUPPLY     0xb4
#define READ_SCRATCHPAD     0xbe

/* supported DS therms families */
#define DS18S20             0x10
#define DS1822              0x22
#define DS18B20             0x28
#define DS1825              0x3b
#define DS28EA00            0x42

/* returns NULL if not supported */
#define dsthName(_id) \
    ((_id[0])==DS18S20 ? "DS18S20" : \
    ((_id[0])==DS1822 ? "DS1822" : \
    ((_id[0])==DS18B20 ? "DS18B20" : \
    ((_id[0])==DS1825 ? "DS1825" : \
    ((_id[0])==DS28EA00 ? "DS28EA00" : \
    (NULL))))))

#ifdef PWR_CTRL_PIN
static OneWireNg *ow = new OneWireNg_CurrentPlatform(OW_PIN, PWR_CTRL_PIN, false);
#else
static OneWireNg *ow = new OneWireNg_CurrentPlatform(OW_PIN, false);
#endif

static void printId(const OneWireNg::Id& id)
{
    const char *name = dsthName(id);

    Serial.print(id[0], HEX);
    for (size_t i=1; i < sizeof(OneWireNg::Id); i++) {
        Serial.print(':');
        Serial.print(id[i], HEX);
    }
    if (name) {
        Serial.print(" -> ");
        Serial.print(name);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    OneWireNg::Id id;
    OneWireNg::ErrorCode ec;

    ow->searchReset();

    do
    {
        ec = ow->search(id);
        if (ec == OneWireNg::EC_MORE || ec == OneWireNg::EC_DONE)
            printId(id);

        /* start temperature conversion */
        ow->addressSingle(id);
        ow->writeByte(CONVERT_T);

#ifdef PARASITE_POWER
        /* power the bus until the next activity on it */
        ow->powerBus(true);
#endif
        delay(750);

        uint8_t touchScrpd[] = {
            READ_SCRATCHPAD,
            /* the read scratchpad will be placed here (9 bytes) */
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };

        ow->addressSingle(id);
        ow->touchBytes(touchScrpd, sizeof(touchScrpd));
        uint8_t *scrpd = &touchScrpd[1];  /* scratchpad data */

        Serial.print("  Scratchpad:");
        for (size_t i = 0; i < sizeof(touchScrpd)-1; i++) {
            Serial.print(!i ? ' ' : ':');
            Serial.print(scrpd[i], HEX);
        }
        Serial.println();

        if (OneWireNg::crc8(scrpd, 8) != scrpd[8]) {
            Serial.println("  Invalid CRC!");
            continue;
        }

        long temp = ((long)(int8_t)scrpd[1] << 8) | scrpd[0];

        if (id[0] != DS18S20) {
            unsigned res = (scrpd[4] >> 5) & 3;
            temp = (temp >> (3-res)) << (3-res);  /* zeroed undefined bits */
            temp = (temp*1000)/16;
        } else
        if (scrpd[7]) {
            temp = 1000*(temp >> 1) - 250;
            temp += 1000*(scrpd[7] - scrpd[6]) / scrpd[7];
        } else {
            /* shall never happen */
            temp = (temp*1000)/2;
            Serial.println("  Zeroed COUNT_PER_C detected!");
        }

        Serial.print("  Temp: ");
        if (temp < 0) {
            temp = -temp;
            Serial.print('-');
        }
        Serial.print(temp / 1000);
        Serial.print('.');
        Serial.print(temp % 1000);
        Serial.println(" C");

    } while (ec == OneWireNg::EC_MORE);

    Serial.println("----------");
    delay(1000);
}

/*
 * Copyright (c) 2022 Piotr Stolarz
 * OneWireNg: Arduino 1-wire service library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

/**
 * DS2431 EEPROM usage example (ESP-IDF).
 *
 * Required configuration:
 * - @c CONFIG_SEARCH_ENABLED,
 * - @c CONFIG_CRC16_ENABLED,
 * - @c CONFIG_MAX_SEARCH_FILTERS >= 1,
 * - @c CONFIG_OVERDRIVE_ENABLED if @c CONFIG_USE_OD_MODE is configured.
 */
#include <stdio.h>
#include <string.h>

#include "OneWireNg_CurrentPlatform.h"
#include "platform/Platform_Delay.h"

#ifndef CONFIG_OW_PIN
# error "CONFIG_OW_PIN is required"
#endif

/* DS2431 family code */
#define DS2431      0x2D

/* memory function commands */
#define CMD_WRITE_SCRATCHPAD    0x0F
#define CMD_COPY_SCRATCHPAD     0x55
#define CMD_READ_SCRATCHPAD     0xAA
#define CMD_READ_MEMORY         0xF0

/* EEPROM row size */
#define DS2431_ROW_SIZE    8
/* EEPROM page size */
#define DS2431_PAGE_SIZE   (4 * DS2431_ROW_SIZE)
/* DS2431 memory size */
#define DS2431_MEM_SIZE    (18 * DS2431_ROW_SIZE)

#if !CONFIG_SEARCH_ENABLED
# error "CONFIG_SEARCH_ENABLED is required"
#endif

#if !CONFIG_CRC16_ENABLED
# error "CONFIG_CRC16_ENABLED is required"
#endif

#if (CONFIG_MAX_SEARCH_FILTERS < 1)
# error "CONFIG_MAX_SEARCH_FILTERS >= 1 is required"
#endif

#if defined(CONFIG_USE_OD_MODE) && !CONFIG_OVERDRIVE_ENABLED
# error "CONFIG_OVERDRIVE_ENABLED is required if CONFIG_USE_OD_MODE is configured"
#endif

static OneWireNg *ow = NULL;

static void printId(const OneWireNg::Id& id)
{
    for (size_t i = 0; i < sizeof(OneWireNg::Id); i++)
        printf("%s%02X", (!i ? "" : ":"), id[i]);

    printf("\n");
}

/**
 * Prints device EEPROM memory on serial. The memory is ready via READ_MEMORY
 * (0xF0) command which doesn't incorporate CRC protection. Therefore usage of
 * the function is problematic on transmission error vulnerable environments
 * (including OD mode).
 *
 * If @c id is NULL the routine resumes communication with lastly addressed
 * device.
 */
static void printMem(const OneWireNg::Id *id)
{
    uint8_t cmd[DS2431_MEM_SIZE + 3];

    cmd[0] = CMD_READ_MEMORY;
    /* start reading from 0x0000 */
    cmd[1] = 0x00; /* TA1 (LSB) */
    cmd[2] = 0x00; /* TA2 (MSB) */

    /* read memory will be placed here */
    uint8_t *mem = &cmd[3];
    memset(mem, 0xff, DS2431_MEM_SIZE);

    if (id) {
        ow->addressSingle(*id);
    } else {
        ow->resume();
    }
    ow->touchBytes(cmd, DS2431_MEM_SIZE + 3);

    for (int i = 0; i < DS2431_MEM_SIZE; i++)
    {
        if (!(i % DS2431_ROW_SIZE)) {
            printf("%02X ", (uint8_t)i);
        }
        printf("%02X", mem[i]);

        if (((i+1) % DS2431_ROW_SIZE) != 0) {
            printf(":");
        } else {
            switch (i/DS2431_ROW_SIZE)
            {
            case 0:
                printf(" Data Memory Page 0\n");
                break;
            case 4:
                printf(" Data Memory Page 1\n");
                break;
            case 8:
                printf(" Data Memory Page 2\n");
                break;
            case 12:
                printf(" Data Memory Page 3\n");
                break;
            case 16:
                printf(" Control Bytes: PCB0:PCB1:PCB2:PCB3:CPB:FACT:USR1:USR2\n");
                break;
            case 17:
                printf(" Reserved\n");
                break;
            default:
                printf("\n");
                break;
            }
        }
    }
}

#ifdef CONFIG_WRITE_DEMO
/**
 * Write a single EEPROM row passed in @c rowData. Row address (0-17) passed
 * by @c rowAddr.
 *
 * If @c checkDataIntegr is @c true scratchpad data are verified against
 * the data being set (@c rowData) if both are the same. This parameter shall
 * be set to @c false for EPROM mode (logical AND performed by write).
 *
 * If @c id is NULL the routine resumes communication with lastly addressed
 * device.
 *
 * On success the routine returns @c true.
 */
static bool writeRow(const OneWireNg::Id *id,
    unsigned rowAddr, const uint8_t rowData[DS2431_ROW_SIZE],
    bool checkDataIntegr)
{
    if (rowAddr > 17) return false;

    uint8_t cmd[DS2431_ROW_SIZE + 6];

    uint8_t ta1 = (rowAddr * DS2431_ROW_SIZE); /* TA1 (LSB) */
    uint8_t ta2 = 0x00;                 /* TA2 (MSB) */

    /* STEP 1: write row data into scratchpad
     */
    cmd[0] = CMD_WRITE_SCRATCHPAD;
    cmd[1] = ta1;
    cmd[2] = ta2;
    memcpy(&cmd[3], rowData, DS2431_ROW_SIZE);

    /* inverted CRC-16 will be placed here */
    uint8_t *crc16 = &cmd[DS2431_ROW_SIZE + 3];
    crc16[0] = 0xff;
    crc16[1] = 0xff;

    if (id) {
        ow->addressSingle(*id);
    } else {
        ow->resume();
    }
    ow->touchBytes(cmd, DS2431_ROW_SIZE + 5);

    if (ow->checkInvCrc16(cmd, DS2431_ROW_SIZE + 3, ow->getLSB_u16(crc16)) !=
        OneWireNg::EC_SUCCESS)
    {
        printf("WRITE SCRATCHPAD: CRC error\n");
        return false;
    }

    /* STEP 2: verify scratchpad integrity
     */
    cmd[0] = CMD_READ_SCRATCHPAD;
    /* TA1, TA2, E/S, row data, CRC16 will be placed here */
    memset(&cmd[1], 0xff, DS2431_ROW_SIZE + 5);

    ow->resume();
    ow->touchBytes(cmd, DS2431_ROW_SIZE + 6);

    crc16 = &cmd[DS2431_ROW_SIZE + 4];
    if (ow->checkInvCrc16(cmd, DS2431_ROW_SIZE + 4, ow->getLSB_u16(crc16)) !=
        OneWireNg::EC_SUCCESS)
    {
        printf("READ SCRATCHPAD: CRC error\n");
        return false;
    }

    /* PF and AA flags must be cleared TA1, TA2 must match */
    uint8_t es = cmd[3];
    if (ta1 != cmd[1] || ta2 != cmd[2] || (es & 0x20) || (es & 0x80))
    {
        printf("READ SCRATCHPAD: command status error\n");
        return false;
    }

    /* check if data was set in scratchpad */
    if (checkDataIntegr && memcmp(&cmd[4], rowData, DS2431_ROW_SIZE))
    {
        printf("READ SCRATCHPAD: row is write protected\n");
        return false;
    }

    /* STEP 3: copy scratchpad into EEPROM
     */
    cmd[0] = CMD_COPY_SCRATCHPAD;
    /* TA1, TA2, E/S already set as required */

    ow->resume();
    ow->touchBytes(cmd, 4);

    /* wait for completion (10 ms) */
    delayMs(10);

    return true;
}

/**
 * @ref writeRow() analogous to write EEPROM page. Page specified by
 * @c pageAddr (0-3).
 */
static bool writePage(const OneWireNg::Id *id,
    unsigned pageAddr, const uint8_t pageData[DS2431_PAGE_SIZE],
    bool checkDataIntegr)
{
    if (pageAddr > 3) return false;

    int i;
    uint8_t row = pageAddr * (DS2431_PAGE_SIZE / DS2431_ROW_SIZE);

    for (i = 0; i < (DS2431_PAGE_SIZE / DS2431_ROW_SIZE); i++, row++) {
        if (!writeRow((!i ? id : NULL),
            row, &pageData[i * DS2431_ROW_SIZE], checkDataIntegr)) break;
    }

    if (i < (DS2431_PAGE_SIZE / DS2431_ROW_SIZE))
    {
        printf("Error writing row %u in page %u\n", (unsigned)row, pageAddr);
        return false;
    }
    return true;
}
#endif /* CONFIG_WRITE_DEMO */

void setup()
{
    OneWireNg::Id id;

    /* id of a DS2431 device for write demo;
       if not set 1st available DS2431 device will be chosen */
    OneWireNg::Id dev = {};

    ow = new OneWireNg_CurrentPlatform(CONFIG_OW_PIN, false);

#ifdef CONFIG_USE_OD_MODE
    ow->overdriveAll();
#endif

    /* search for DS2431 devices connected to the bus
     */
    ow->searchFilterAdd(DS2431);
    printf("Connected DS2431 devices:\n");

    ow->searchReset();
    while (ow->search(id) == OneWireNg::EC_MORE) {
        if (dev[0] != DS2431)
            memcpy(&dev, &id[0], sizeof(OneWireNg::Id));

        printId(id);
        printMem(NULL);

        printf("----------\n");
    }

#ifdef CONFIG_WRITE_DEMO
    /* if no DS2431 found finish the demo */
    if (dev[0] != DS2431) return;

    uint8_t pageData[DS2431_PAGE_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
        0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
    };
    if (writePage(&dev, 1, pageData, true)) {
        printf("Page successfully written to EEPROM\n");
    }
#endif
}

void loop()
{
    delayMs(1000);
}

extern "C" void app_main()
{
    setup();
    for (;;)
        loop();
}

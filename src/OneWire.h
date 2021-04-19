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

#ifndef __OWNG_ONEWIRE__
#define __OWNG_ONEWIRE__

#include <new>       /* in-place new-operator */
#include <stdint.h>
#include <string.h>  /* memcpy */
#include "OneWireNg_CurrentPlatform.h"

/**
 * OneWire library compatibility interface class.
 *
 * This class implements classic Arduino OneWire library interface using
 * OneWireNg service API. The main purpose of this class is to provide fast
 * and effortless mechanism for developers experiencing issues with OneWire
 * and eager to give OneWireNg a try. Finally, it's strongly recommended to
 * switch into OneWireNg interface rather than stay with the OneWire due to
 * OneWireNg's more mature and feature-rich API (search filtering, OD mode,
 * touch support).
 *
 * For OneWire API specification refer to its source repository:
 * https://github.com/PaulStoffregen/OneWire
 *
 * NOTE: Although the OneWire API have been strictly preserved there are some
 * implementation differences between them. The most fundamental one is probably
 * the "power" argument in write family of functions. OneWireNg allows providing
 * power to the 1-wire bus only in specific conditions and the bus is always
 * "de-powered" before next activity on it. On the other hand OneWire requires
 * de-powering each time the power has been provided to the bus and 1-wire
 * activities doesn't turn them off. OneWireNg doesn't emulate this buggy
 * behavior.
 */
class OneWire
{
public:
    /*
     * NOTE: The default (no-args) constructor requires begin() method to be
     * called subsequently with MCU's GPIO pin number controlling the 1-wire
     * data bus as an argument. Any attempt to call OneWire API w/o this
     * begin() call will cause NULL pointer dereference inside OneWire logic.
     */
    OneWire():
        _srch_done(false), _ow(NULL)
    {}

    OneWire(uint8_t pin):
        _srch_done(false), _ow(NULL)
    {
        begin(pin);
    }

    void begin(uint8_t pin)
    {
        if (_ow)
            _ow->OneWireNg::~OneWireNg();

        _ow = new (_ow_buf) OneWireNg_CurrentPlatform(
            pin,
            false   /* turn off internal pull-up */
        );
    }

    uint8_t reset(void) {
        return (uint8_t)(_ow->reset() == OneWireNg::EC_SUCCESS);
    }

    void select(const uint8_t rom[8]) {
        _ow->writeByte(OneWireNg::CMD_MATCH_ROM);
        _ow->writeBytes(&rom[0], 8);
    }

    void skip(void) {
        _ow->writeByte(OneWireNg::CMD_SKIP_ROM);
    }

    void write(uint8_t v, uint8_t power = 0) {
        _ow->writeByte(v);
        if (power)
            _ow->powerBus(true);
    }

    void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0) {
        _ow->writeBytes(buf, count);
        if (power)
            _ow->powerBus(true);
    }

    uint8_t read(void) {
        return _ow->readByte();
    }

    void read_bytes(uint8_t *buf, uint16_t count) {
        _ow->readBytes(buf, count);
    }

    void write_bit(uint8_t v) {
        _ow->writeBit(v);
    }

    uint8_t read_bit(void) {
        return (uint8_t)_ow->readBit();
    }

    void depower(void) {
        _ow->powerBus(false);
    }

    void reset_search() {
        _srch_done = false;
        _ow->searchReset();
    }

    bool search(uint8_t *newAddr, bool search_mode = true) {
        OneWireNg::Id id;

        if (!_srch_done) {
            OneWireNg::ErrorCode ec = _ow->search(id, !search_mode);
            memcpy(newAddr, &id[0], sizeof(id));

            _srch_done = (ec == OneWireNg::EC_DONE);
            return (ec == OneWireNg::EC_MORE || _srch_done);
        }
        return false;
    }

    static uint8_t crc8(const uint8_t *addr, uint8_t len) {
        return OneWireNg::crc8(addr, len);
    }

#ifdef CONFIG_CRC16_ENABLED
    static bool check_crc16(const uint8_t *input, uint16_t len,
        const uint8_t *inverted_crc, uint16_t crc = 0)
    {
        return !(uint16_t)(crc16(input, len, crc) ^
            ~OneWireNg::getLSB_u16(inverted_crc));
    }

    static uint16_t crc16(
        const uint8_t *input, uint16_t len, uint16_t crc = 0)
    {
        return OneWireNg::crc16(input, len, crc);
    }
#endif

private:
    bool _srch_done;
    uint8_t _ow_buf[sizeof(OneWireNg_CurrentPlatform)];
    OneWireNg *_ow;
};

#endif  /* __OWNG_ONEWIRE__ */

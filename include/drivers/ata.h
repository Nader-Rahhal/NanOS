#pragma once

#include <stdint.h>
#include "drivers/serial.h"
#include "util.h"

#define ATA_PRIMARY_IO_BASE 0x1F0
#define ATA_SECONDARY_IO_BASE 0x170

#define ATA_PRIMARY_DATA_PORT (ATA_PRIMARY_IO_BASE + 0)
#define ATA_PRIMARY_ERROR_PORT (ATA_PRIMARY_IO_BASE + 1)
#define ATA_PRIMARY_SECTOR_COUNT_PORT (ATA_PRIMARY_IO_BASE + 2)
#define ATA_PRIMARY_SECTOR_NUMBER_PORT (ATA_PRIMARY_IO_BASE + 3)
#define ATA_PRIMARY_CYLINDER_LOW_PORT (ATA_PRIMARY_IO_BASE + 4)
#define ATA_PRIMARY_CYLINDER_HIGH_PORT (ATA_PRIMARY_IO_BASE + 5)
#define ATA_PRIMARY_DRIVE_HEAD_PORT (ATA_PRIMARY_IO_BASE + 6)
#define ATA_PRIMARY_STATUS_PORT (ATA_PRIMARY_IO_BASE + 7)
#define ATA_PRIMARY_COMMAND_PORT (ATA_PRIMARY_IO_BASE + 7)

#define ATA_PRIMARY_CTRL_BASE 0x3F6
#define ATA_PRIMARY_ALT_STATUS_PORT (ATA_PRIMARY_CTRL_BASE + 0)
#define ATA_PRIMARY_DEVICE_CONTROL_PORT (ATA_PRIMARY_CTRL_BASE + 0)

#define IDENTIFY_COMMAND 0xEC
#define CACHE_FLUSH_COMMAND 0xE7
#define READ_SECTORS_COMMAND 0x20
#define WRITE_SECTORS_COMMAND 0x30

#define BYTES_PER_SECTOR 512

namespace ata {

inline uint16_t identify_data[256];

// Waits for BSY to clear and DRQ to set — use for commands that transfer data
// (IDENTIFY, READ SECTORS, WRITE SECTORS).
inline bool poll_data_ready() {
    uint8_t status = util::inb(ATA_PRIMARY_STATUS_PORT);

    while (status & 0x80) { // BSY
        status = util::inb(ATA_PRIMARY_STATUS_PORT);
    }

    if (status & 0x01) { // ERR
        uint8_t err = util::inb(ATA_PRIMARY_ERROR_PORT);
        serial::print("ATA error during poll\n");
        return false;
    }

    if (!(status & 0x08)) { // DRQ
        serial::print("ATA device not ready\n");
        return false;
    }

    return true;
}

// Waits for BSY to clear only — use for commands with no data phase
// (FLUSH CACHE, etc). Does NOT check DRQ.
inline bool poll_command_complete() {
    uint8_t status = util::inb(ATA_PRIMARY_STATUS_PORT);

    while (status & 0x80) { // BSY
        status = util::inb(ATA_PRIMARY_STATUS_PORT);
    }

    if (status & 0x01) { // ERR
        uint8_t err = util::inb(ATA_PRIMARY_ERROR_PORT);
        serial::print("ATA error during poll\n");
        return false;
    }

    return true;
}

inline void init() {
    util::outb(ATA_PRIMARY_DRIVE_HEAD_PORT, 0xA0);

    util::io_wait();
    util::io_wait();
    util::io_wait();
    util::io_wait();

    uint8_t status = util::inb(ATA_PRIMARY_STATUS_PORT);
    if (status == 0xFF) {
        serial::print("No ATA device found (floating bus)\n");
        return;
    }

    util::outb(ATA_PRIMARY_COMMAND_PORT, IDENTIFY_COMMAND);
    status = util::inb(ATA_PRIMARY_STATUS_PORT);

    if (status == 0) {
        serial::print("No ATA device found\n");
        return;
    }

    if (!poll_data_ready()) {
        serial::print("ATA IDENTIFY failed\n");
        return;
    }

    for (int i = 0; i < BYTES_PER_SECTOR / 2; i++) {
        identify_data[i] = util::inw(ATA_PRIMARY_DATA_PORT);
    }

    serial::print("ATA device initialized\n");
}

inline void select_lba(uint32_t lba) {
    util::outb(ATA_PRIMARY_DRIVE_HEAD_PORT, 0xE0 | ((lba >> 24) & 0x0F)); // LBA mode
    util::io_wait();
    util::io_wait();
    util::io_wait();
    util::io_wait();
}

inline void write_sector(uint32_t lba, uint16_t* buffer) {
    util::outb(ATA_PRIMARY_SECTOR_COUNT_PORT, 1);
    util::outb(ATA_PRIMARY_SECTOR_NUMBER_PORT, (uint8_t)(lba & 0xFF));
    util::outb(ATA_PRIMARY_CYLINDER_LOW_PORT, (uint8_t)((lba >> 8) & 0xFF));
    util::outb(ATA_PRIMARY_CYLINDER_HIGH_PORT, (uint8_t)((lba >> 16) & 0xFF));

    select_lba(lba);

    util::outb(ATA_PRIMARY_COMMAND_PORT, WRITE_SECTORS_COMMAND);

    if (!poll_data_ready()) {
        serial::print("ATA write failed\n");
        return;
    }

    for (int i = 0; i < BYTES_PER_SECTOR / 2; i++) {
        util::outw(ATA_PRIMARY_DATA_PORT, buffer[i]);
    }

    util::outb(ATA_PRIMARY_COMMAND_PORT, CACHE_FLUSH_COMMAND);
    if (!poll_command_complete()) {
        serial::print("ATA cache flush failed\n");
        return;
    }
}

inline void read_sector(uint32_t lba, uint16_t* buffer) {
    util::outb(ATA_PRIMARY_SECTOR_COUNT_PORT, 1);
    util::outb(ATA_PRIMARY_SECTOR_NUMBER_PORT, (uint8_t)(lba & 0xFF));
    util::outb(ATA_PRIMARY_CYLINDER_LOW_PORT, (uint8_t)((lba >> 8) & 0xFF));
    util::outb(ATA_PRIMARY_CYLINDER_HIGH_PORT, (uint8_t)((lba >> 16) & 0xFF));

    select_lba(lba);

    util::outb(ATA_PRIMARY_COMMAND_PORT, READ_SECTORS_COMMAND);

    if (!poll_data_ready()) {
        serial::print("ATA read failed\n");
        return;
    }

    for (int i = 0; i < BYTES_PER_SECTOR / 2; i++) {
        buffer[i] = util::inw(ATA_PRIMARY_DATA_PORT);
    }
}

}
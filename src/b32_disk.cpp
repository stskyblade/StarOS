#include "bootloader32.h"

// https://wiki.osdev.org/ATA_PIO_Mode#Registers
const uint16_t DataRegister = 0;
const uint16_t SectorCountRegister = 2;
const uint16_t LBAloRegister = 3;
const uint16_t LBAmidRegister = 4;
const uint16_t CylinderHighRegister = 5;
const uint16_t DriveRegister = 6;
const uint16_t StatusRegister = 7;
const uint16_t CommandRegister = 7;

int detect_ide_disk(uint16_t port_base, bool is_master) {
    // https://wiki.osdev.org/ATA_PIO_Mode#Floating_Bus
    // read Status byte, 0xff is invalid value
    uint8_t data = inb(port_base + StatusRegister);
    if (data == 0xff) {
        return false;
    }

    // random value write & read test
    data = 0xab; // random value 1
    outb(port_base + SectorCountRegister, data);
    uint8_t data_read = inb(port_base + SectorCountRegister);
    if (data != data_read) {
        return false;
    }

    data = 0x35; // random value 2
    outb(port_base + CylinderHighRegister, data);
    data_read = inb(port_base + CylinderHighRegister);
    if (data != data_read) {
        return false;
    }

    // select drive, 400ns
    outb(port_base + DriveRegister, 0b10100000 | ((!is_master) << 4));
    for (int i = 0; i < 15; i++) {
        data = inb(port_base + StatusRegister);
    }

    // IDENTIFY test
    outb(port_base + SectorCountRegister, 0);
    outb(port_base + LBAloRegister, 0);
    outb(port_base + LBAmidRegister, 0);
    outb(port_base + CylinderHighRegister, 0);
    outb(port_base + CommandRegister, 0xEC); // IDENTIFY command
    data = inb(port_base + StatusRegister);
    if (data == 0) {
        // driver doesn't exist
        return false;
    }

    uint8_t mid_data = inb(port_base + LBAmidRegister);
    uint8_t hi_data = inb(port_base + CylinderHighRegister);
    if (mid_data && hi_data) {
        // not ATA device

        if (mid_data == 0x14 && hi_data == 0xeb) {
            // ATAPI device
        } else if (mid_data == 0x3c && hi_data == 0xc3) {
            // SATA device
        } else if (mid_data == 0x69 && hi_data == 0x96) {
            // SATAPI device
        }

        return false;
    }

    // if BSY bit set, poll
    while (data & 0x80) {
        mid_data = inb(port_base + LBAmidRegister);
        hi_data = inb(port_base + CylinderHighRegister);
        if (mid_data && hi_data) {
            // not ATA device
            return false;
        }

        data = inb(port_base + StatusRegister);
    }

    // poll until DRQ or ERR
    const uint8_t DRQ = 0x8;
    const uint8_t ERR = 0x1;
    while ((data & (DRQ | ERR)) == 0) {
        data = inb(port_base + StatusRegister);
    }

    if (data & ERR) {
        return false;
    }

    // DRQ
    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        uint16_t word = inw(port_base + DataRegister);
        buffer[i] = word;
    }

    bool is_harddisk = buffer[0];
    bool is_support_lba48 = buffer[83] & (1 << 10);
    uint32_t *buffer32 = (uint32_t *)buffer;
    uint32_t sector_count = buffer32[30];
    bool is_support_lba28 = sector_count != 0;
    uint64_t *buffer64 = (uint64_t *)buffer;
    uint64_t sector_count_48 = buffer64[25];

    return true;
}

int read_disk_sector(uint64_t sector_number, uint64_t count, uint8_t *dest) {
    bool status = detect_ide_disk(0x1f0, true);
}
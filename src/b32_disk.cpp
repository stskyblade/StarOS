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

const uint8_t DRQ = 0x8;
const uint8_t ERR = 0x1;

const char *error_msg = "";

// return true on success
bool detect_ide_disk(uint16_t port_base, bool is_master) {
    // https://wiki.osdev.org/ATA_PIO_Mode#Floating_Bus
    // read Status byte, 0xff is invalid value
    uint8_t data = inb(port_base + StatusRegister);
    if (data == 0xff) {
        error_msg = "Floating bus test failed.";
        return false;
    }

    // random value write & read test
    data = 0xab; // random value 1
    outb(port_base + SectorCountRegister, data);
    uint8_t data_read = inb(port_base + SectorCountRegister);
    if (data != data_read) {
        error_msg = "Random value test 1 failed.";
        return false;
    }

    data = 0x35; // random value 2
    outb(port_base + CylinderHighRegister, data);
    data_read = inb(port_base + CylinderHighRegister);
    if (data != data_read) {
        error_msg = "Random value test 2 failed.";
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
        error_msg = "Drive doesn't exist.";
        return false;
    }

    uint8_t mid_data = inb(port_base + LBAmidRegister);
    uint8_t hi_data = inb(port_base + CylinderHighRegister);
    if (mid_data && hi_data) {
        // not ATA device

        if (mid_data == 0x14 && hi_data == 0xeb) {
            // ATAPI device
            error_msg = "ATAPI device.";
        } else if (mid_data == 0x3c && hi_data == 0xc3) {
            // SATA device
            error_msg = "SATA device.";
        } else if (mid_data == 0x69 && hi_data == 0x96) {
            // SATAPI device
            error_msg = "SATAPI device.";
        } else {
            error_msg = "unknown device.";
        }

        return false;
    }

    // if BSY bit set, poll
    while (data & 0x80) {
        mid_data = inb(port_base + LBAmidRegister);
        hi_data = inb(port_base + CylinderHighRegister);
        if (mid_data && hi_data) {
            // not ATA device
            error_msg = "not ATA device.";
            return false;
        }

        data = inb(port_base + StatusRegister);
    }

    // poll until DRQ or ERR

    while ((data & (DRQ | ERR)) == 0) {
        data = inb(port_base + StatusRegister);
    }

    if (data & ERR) {
        error_msg = "ERR flags set";
        return false;
    }

    // DRQ
    uint16_t buffer[256];
    uint16_t word = 0;
    for (int i = 0; i < 256; i++) {
        word = inw(port_base + DataRegister);
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

// https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
// return 0 on success, -1 on failure
int read_ide_disk_sector(uint16_t port_base, bool is_master, uint64_t lba, uint64_t count, uint8_t *dest) {
    // select drive and set high 4 bit LBA
    outb(port_base + DriveRegister, 0xe0 | ((!is_master) << 4) | ((lba >> 24) & 0x0f));
    uint8_t data = 0;

    for (int i = 0; i < 15; i++) {
        inb(port_base + StatusRegister);
    }

    // FIXME: count > 256
    // send address
    outb(port_base + SectorCountRegister, count);
    outb(port_base + LBAloRegister, lba & 0xff);
    outb(port_base + LBAmidRegister, lba >> 8);
    outb(port_base + CylinderHighRegister, lba >> 16);
    outb(port_base + CommandRegister, 0x20); // send READ command

    // poll
    data = inb(port_base + StatusRegister);
    while (data & 0x80) {
        // if BSY set, poll
        data = inb(port_base + StatusRegister);
    }

    while ((data & (DRQ | ERR)) == 0) {
        data = inb(port_base + StatusRegister);
    }

    if (data & DRQ) {
        // read data
        uint16_t *buffer = (uint16_t *)dest;
        uint16_t word = 0;
        for (int i = 0; i < 256; i++) {
            word = inw(port_base + DataRegister);
            buffer[i] = word;
        }
        return 0;
    }

    return -1;
}

// return 0 on success
int read_disk_sector(uint64_t sector_number, uint64_t count, uint8_t *dest) {
    uint16_t port_base = 0x1f0;
    bool is_master = true;
    bool success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    success ? printf("true ") : printf("false ");

    if (success) {
        int error = read_ide_disk_sector(port_base, is_master, sector_number, count, dest);
    } else {
        printf(error_msg);
    }
    printf("\n");

    // test other disk drive
    port_base = 0x1f0;
    is_master = false;
    success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    success ? printf("true ") : printf("false ");

    if (success) {
    } else {
        printf(error_msg);
    }
    printf("\n");

    port_base = 0x170;
    is_master = true;
    success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    success ? printf("true ") : printf("false ");

    if (success) {
    } else {
        printf(error_msg);
    }
    printf("\n");

    port_base = 0x170;
    is_master = false;
    success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    success ? printf("true ") : printf("false ");

    if (success) {
    } else {
        printf(error_msg);
    }
    printf("\n");
}
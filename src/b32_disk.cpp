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

// `bus` for bus number, 8-bit
// `device` for device number, 5-bit
// `func` for function number, 3-bit
// `offset` for byte offset inside 64 32-bit registers, 256 bytes total
uint32_t read_long_PCI_config_space(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = 0;
    address |= 1 << 31; // enable bit
    address |= bus << 16;
    address |= (device & 0b11111) << 11;
    address |= (func & 0b111) << 8;
    address |= offset;

    outl(0xcf8, address);
    uint32_t data = inl(0xcfc);
    return data;
}

void write_long_PCI_config_space(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data) {
    uint32_t address = 0;
    address |= 1 << 31; // enable bit
    address |= bus << 16;
    address |= (device & 0b11111) << 11;
    address |= (func & 0b111) << 8;
    address |= offset;

    outl(0xcf8, address);
    outl(0xcfc, data);
    return;
}

void read_target_config_space(uint8_t bus, uint8_t device, uint8_t func, uint32_t *buf) {
    for (int i = 0; i < 64; i++) {
        buf[i] = read_long_PCI_config_space(bus, device, func, i << 2);
    }
}

int check_target(uint8_t bus, uint8_t device, uint8_t func) {
    uint32_t buf[64];
    read_target_config_space(bus, device, func, buf);

    // check DEVICE id & Vendor id
    if (buf[0] == 0xFFFFFFFF) {
        // printf("non-exist.\n");
        return -1;
    }

    uint32_t data = buf[2];
    uint32_t data2 = buf[3];
    // printf("%x:%x.%x ", bus, device, func);
    uint8_t class_code = data >> 24;
    uint8_t subclass = data >> 16;
    uint8_t prog_if = data >> 8;
    // printf("%x, %x, %x, %x.\n", data >> 24, data >> 16 & 0xFF, data >> 8 & 0xFF, data2 >> 16 & 0xFF);

    // set 0:1F.2 device to compatibility mode
    if (class_code == 1 && subclass == 1 && prog_if == 0x8F) {
        // printf("change IDE controller to compatible mode.\n");
        int new_data = 0;
        new_data |= class_code << 24;
        new_data |= subclass << 16;
        new_data |= (prog_if & 0x8A); // clear bit 0 & bit 2
        new_data |= (data & 0xFF);
        // write back data to buf[2]
        write_long_PCI_config_space(bus, device, func, 2 << 2, new_data);
    }
    return 0;
}

// enumerate all PCI devices and print message
void enumerate_PCI_devices() {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int func = 0; func < 8; func++) {
                check_target(bus, device, func);
            }
        }
    }
}

int detect_ide_disks() {
    uint16_t port_base = 0x1f0;
    bool is_master = true;
    bool success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    is_master ? printf("master ") : printf("slave  ");
    success ? printf("true ") : printf("false ");

    if (success) {
        // int error = read_ide_disk_sector(port_base, is_master, sector_number, count, dest);
    } else {
        printf(error_msg);
    }
    printf("\n");

    // test other disk drive
    port_base = 0x1f0;
    is_master = false;
    success = detect_ide_disk(port_base, is_master);
    printf("Check disk 0x%x ", port_base);
    is_master ? printf("master ") : printf("slave  ");
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
    is_master ? printf("master ") : printf("slave  ");
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
    is_master ? printf("master ") : printf("slave  ");
    success ? printf("true ") : printf("false ");

    if (success) {
    } else {
        printf(error_msg);
    }
    printf("\n");
}

// return 0 on success
int read_disk_sector(uint64_t sector_number, uint64_t count, uint8_t *dest) {
    // enumerate PCI bus
    enumerate_PCI_devices();

    uint16_t port_base = 0x1f0;
    bool is_master = true;
    int error = read_ide_disk_sector(port_base, is_master, sector_number, count, dest);
    printf("is master:\n");
    if (!error) {
        print_memory(dest, 16 * 7);
    }

    is_master = false;
    error = read_ide_disk_sector(port_base, is_master, sector_number, count, dest);
    printf("is slave:\n");
    if (!error) {
        print_memory(dest, 16 * 7);
    }
    return 0;
}
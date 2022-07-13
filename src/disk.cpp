#include "kernel.h"

// Primary/Secondary Bus section
#define ATA_BASE 0x1f0
const uint ATA1_BASE = 0x1f0; // Primary bus of first two buses
const uint ATA2_BASE = 0x170; // Secondary bus of first two buses
const uint ATA3_BASE = 0x1e8; // Primary bus of second two buses
const uint ATA4_BASE = 0x168; // Secondary bus of second two buses
const uint DATA_REG = 0;
const uint ERROR_REG = 1;
const uint Features_REG = 1;
const uint SECTOR_COUNT_REG = 2;
const uint LBA_LOW_REG = 3;
const uint LBA_MID_REG = 4;
const uint LBA_HIGH_REG = 5;
const uint HEAD_REG = 6;
const uint DRIVE_REG = 6;
const uint STATUS_REG = 7;
const uint COMMAND_REG = 7;

bool init_ide() {
    // select
    outb(ATA1_BASE + DRIVE_REG, 0xA0); // select primary bus, aka master device

    // Floating bus test
    uchar data = 0;
    // 400ns delay
    for (int i = 0; i < 16; i++) {
        data = inb(ATA1_BASE + STATUS_REG);
    }

    if (data == 0xff) {
        printf("Floating bus testing failed.");
        return false;
    }

    // random value read/write test
    data = 0xa1;
    outb(ATA1_BASE + SECTOR_COUNT_REG, data);
    uchar tmp = inb(ATA1_BASE + SECTOR_COUNT_REG);
    if (tmp != data) {
        printf("Random value write/read test failed");
    }

    return true;

    // identify command
    outb(ATA1_BASE + SECTOR_COUNT_REG, 0);
    outb(ATA1_BASE + LBA_LOW_REG, 0);
    outb(ATA1_BASE + LBA_MID_REG, 0);
    outb(ATA1_BASE + LBA_HIGH_REG, 0);
    outb(ATA1_BASE + COMMAND_REG, 0xEC);

    tmp = inb(ATA1_BASE + STATUS_REG);

    if (tmp == 0) {
        printf("Identify command test failed");
    }

    while ((tmp & 0x80)) {
        tmp = inb(ATA1_BASE + STATUS_REG);
        ;
    }

    tmp = inb(ATA1_BASE + LBA_MID_REG);
    if (tmp) {
        printf("Identify command test failed");
    }
    tmp = inb(ATA1_BASE + LBA_HIGH_REG);
    if (tmp) {
        printf("Identify command test failed");
    }

    tmp = inb(ATA1_BASE + STATUS_REG);
    while (!(tmp & 0x9)) {
        tmp = inb(ATA1_BASE + STATUS_REG);
    }

    if (!(tmp & 0x1)) {
        ushort value = 0;
        ushort buffer[256];

        for (int i = 0; i < 256; i++) {
            value = inw(ATA1_BASE + DATA_REG);
            buffer[i] = value;
        }

        printf("\nReading result of IDENTIFY command success.\n");
        uchar *p;
        p = (uchar *)buffer;
        for (int i = 0; i < 512; i++) {
            printf("%x", p[i]);
            if (i % 2 == 1) {
                printf(" ");
            }
            if (i % 16 == 15) {
                printf("\n");
            }
        }
    }

    return true;
}

bool read_sector(uint32_t addr, size_t sector_count, uchar *buffer) {
    init_ide();
    // FIXME: assert count <= 255

    // TODO: wait for ready
    uchar data;
    do {
        data = inb(ATA1_BASE + STATUS_REG);
    } while (data & 0x88);

    // how many sector
    data = sector_count;
    outb(ATA1_BASE + SECTOR_COUNT_REG, data);

    // set addr
    data = addr & 0xff;
    outb(ATA_BASE + LBA_LOW_REG, data);
    data = addr >> 8 & 0xff;
    outb(ATA_BASE + LBA_MID_REG, data);
    data = addr >> 16 & 0xff;
    outb(ATA_BASE + LBA_HIGH_REG, data);
    data = (addr >> 24 & 0x0f) | 0xe0; // Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: outb(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
    outb(ATA_BASE + 6, data);

    // read command
    data = 0x20;
    outb(ATA_BASE + COMMAND_REG, data);

    // pool
    data = inb(ATA_BASE + STATUS_REG);
    while ((data & 0x88) != 0x08) {
        data = inb(ATA_BASE + STATUS_REG);
    }

    // read data
    int loop = sector_count * 256;
    ushort d;
    int p = 0;
    //    printf("\nPrint content of data:\n");
    while (loop--) {
        d = inw(ATA_BASE);

        // printf("%x", d);
        // printf("%x", d >> 8);

        buffer[p] = d & 0xff;
        buffer[p + 1] = (d >> 8) & 0xff;

        // printf(" ");

        if (p % 8 == 6) {
            // printf("\n");
        }
        p += 2;
    }
    //    printf("\n");
    return true;
}

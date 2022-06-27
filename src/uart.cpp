#include "kernel.h"

// Serial port
const ushort PORT = 0x3f8; // COM1
const ushort DATA_REG = 0;
const ushort INTERRUPT_ENABLE_REG = 1;
const ushort INTERRUPT_IDENTIFICATION_REG = 2;
const ushort FIFO_CONTROL_REG = 2;
const ushort LINE_CONTROL_REG = 3;
const ushort MODEM_CONTROL_REG = 4;
const ushort LINE_STATUE_REG = 5;
const ushort MODEM_STATUS_REG = 6;
const ushort SCRATCH_REG = 7;

int init_serial() {
    // Read uart 16550 chip data sheet for more details

    outb(PORT + INTERRUPT_ENABLE_REG, 0x00); // Disable all interrupts

    outb(PORT + LINE_CONTROL_REG, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);                // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);                //                  (hi byte)

    outb(PORT + LINE_CONTROL_REG, 0x03);  // 8 bits, no parity, one stop bit
    outb(PORT + FIFO_CONTROL_REG, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + MODEM_CONTROL_REG, 0x0B); // IRQs enabled, RTS/DSR set

    // loopback mode is used to test whether the chip is working
    // data you sent will presents in receiver register
    outb(PORT + MODEM_CONTROL_REG, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + DATA_REG, 0xAE);          // Test serial chip (send byte 0xAE and check if
                                          // serial returns same byte)
    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
    return 0;
}

int is_transmit_empty() {
    return inb(PORT + LINE_STATUE_REG) & 0x20;
}

void write_serial(char a) {
    while (is_transmit_empty() == 0)
        ;

    outb(PORT + DATA_REG, a);
}

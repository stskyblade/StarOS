// This file runs on 32-bit protected mode, but not in kernel
// It's a part of bootloader.

#include "bootloader32.h"
#include <stdarg.h>
#include <stdint.h>

const uint16_t PORT = 0x3f8; // COM1

static int init_serial() {
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00); //                  (hi byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
    return 0;
}

// Write char c to VGA text mode memory at position (row, column)
void output_char(uint8_t c, uint8_t property, uint8_t row, uint8_t column) {
    uint16_t *ptr = (uint16_t *)0xb8000 + row * 80 + column;
    uint16_t data = (property << 8) + c;
    *ptr = data;
    return;
}

uint8_t row = 0;
uint8_t column = 0;
bool is_screen_cleared = false;
bool is_serial_inited = false;

int is_transmit_empty() {
    return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
    while (is_transmit_empty() == 0)
        ;

    outb(PORT, a);
}

// print a char, and increse position
void print_c(char c) {
    write_serial(c);

    if (c == '\n') {
        row++;
        row = row % 25;
        column = 0;

        if (row == 0) {
            sleep(2);
        }
        return;
    }

    output_char(c, 0b00000111, row, column);
    column++;
    if (column == 80) {
        column = 0;
        row++;
    }

    // TODO: row exceed max row 24
    if (row == 25) {
        row = 0;
        sleep(2);
    }
}

// print a 16-bit unsigned `data` in hex representation
// void print_hex(uint32_t data) {
//     if (data == 0) {
//         print_c('0');
//         return;
//     }

//     char *hex_str = "0123456789ABCEDF";
//     char buffer[8];
//     for (int i = 0; i < 8; i++) {
//         int reminder = data % 16;
//         data = data / 16;
//         buffer[7 - i] = hex_str[reminder];
//     }

//     // remove zeros
//     int index = 0;
//     while (buffer[index] == '0') {
//         index++;
//     }

//     for (int i = index; i < 8; i++) {
//         print_c(buffer[i]);
//     }
// }

void print_memory(uint8_t *buf, int length) {
    uint16_t *word_buf = (uint16_t *)buf;
    for (int i = 0; i < length / 2; i++) {
        print_hex(word_buf[i]);
        if (i % 8 == 7) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

// Same as C function printf
void printf(const char *restrict, ...) {
    va_list args;
    va_start(args, restrict);

    if (!is_screen_cleared) {
        for (int row = 0; row < 25; row++) {
            for (int col = 0; col < 80; col++) {
                output_char(' ', 0b00000000, row, col);
            }
        }
        is_screen_cleared = true;
    }

    if (!is_serial_inited) {
        init_serial();
        is_serial_inited = true;
    }

    for (const char *p = restrict; *p != '\0'; p++) {
        char c = *p;
        if (c == '%') {
            p++;
            c = *p;
            if (c == 'x') {
                uint32_t data = va_arg(args, int);
                // in hex representation
                print_hex(data);
            } else if (c == 's') {
                // print string
                const char *s = va_arg(args, char *);
                printf(s);
            } else if (c == 'd') {
                int data = va_arg(args, int);
                print_int(data);
            }
        } else {
            print_c(c);
        }
    }

    va_end(args);
}
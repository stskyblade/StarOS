// This file runs on 32-bit protected mode, but not in kernel
// It's a part of bootloader.

#include <stdarg.h>
#include <stdint.h>

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

// print a char, and increse position
void print_c(char c) {
    if (c == '\n') {
        row++;
        row = row % 25;
        column = 0;
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
    }
}

// print a 16-bit unsigned `data` in hex representation
void print_hex(uint32_t data) {
    char *hex_str = "0123456789ABCEDF";
    char buffer[8];
    for (int i = 0; i < 8; i++) {
        int reminder = data % 16;
        data = data / 16;
        buffer[7 - i] = hex_str[reminder];
    }
    for (int i = 0; i < 8; i++) {
        print_c(buffer[i]);
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

    for (const char *p = restrict; *p != '\0'; p++) {
        char c = *p;
        if (c == '%') {
            p++;
            c = *p;
            if (c == 'x') {
                // length modifier
                int data = va_arg(args, int);
                // in hex representation
                print_hex(data);
            }
        } else {
            print_c(c);
        }
    }

    va_end(args);
}
// This file runs on 32-bit protected mode, but not in kernel
// It's a part of bootloader.

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

// Same as C function printf
void printf(const char *restrict, ...) {
    if (!is_screen_cleared) {
        for (int row = 0; row < 25; row++) {
            for (int col = 0; col < 80; col++) {
                output_char(' ', 0b00000000, row, col);
            }
        }
        is_screen_cleared = true;
    }

    for (const char *p = restrict; *p != '\0'; p++) {
        print_c(*p);
    }
}
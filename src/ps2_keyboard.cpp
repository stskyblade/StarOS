#include "bootloader32.h"
#include "kernel.h"

uint8_t scan_code_table[] = {0,
                             0, // escape
                             '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                             0,   0,   0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y',
                             'U', 'I', 'O', 'P', 0,   0,   0,   0,   'A', 'S',
                             'D', 'F', 'G', 'H', 'J', 'K', 'L', 0,   0,   0,
                             0,   0,   'Z', 'X', 'C', 'V', 'B', 'N', 'M'};

// return true if data <= right && data >= left
bool in_range(uint64_t data, uint32_t left, uint32_t right) {
    return (data >= left) && (data <= right);
}

bool is_complete_scancode(uint64_t code) {
    // scan_code_1.txt

    // single byte
    if (code <= 0xFF) {
        if (in_range(code, 0x01, 0x53)) {
            return true;
        }
        if (in_range(code, 0x57, 0x58)) {
            return true;
        }
        if (in_range(code, 0x81, 0xd3)) {
            return true;
        }
        if (in_range(code, 0xd7, 0xd8)) {
            return true;
        }
        return false;
    }

    // two bytes
    if (code <= 0xFFFF) {
        // not accurate, including invalid scan codes
        uint32_t high_byte = (code & 0xff00) >> 8;
        uint32_t low_byte = code & 0xff;
        debug("expr1 %d", high_byte == 0xe0);
        debug("expr2 %d", low_byte != 0x2a && low_byte != 0xb7);
        if (high_byte == 0xE0 && (low_byte != 0x2A && low_byte != 0xB7)) {
            return true;
        }
        return false;
    }

    // four bytes
    if (code <= 0xFFFFFFFF) {
        return code == 0xe02ae037 || code == 0xe0b7e0aa;
    }

    // six bytes
    if (code <= 0xFFFFFFFFFFFF) {
        auto result = code == 0xe11d45e19dc5;

        uint32_t high_four_bytes = code >> 32;
        uint32_t low_four_bytes = code;
        return result;
    }
    return false;
}

void response_scancode(uint64_t code) {
    uint32_t high_four_bytes = code >> 32;
    uint32_t low_four_bytes = code;
    debug("scan_code: 0x%x%x", high_four_bytes, low_four_bytes);
}

uint64_t SCAN_CODE = 0;
void ps2_keyboard_interrupt() {
    uint32_t data = inb(KEYBOARD_DATA_PORT);
    outb(0x20, 0x20); // send EOI to PIC interrupt controller

    SCAN_CODE = (SCAN_CODE << 8) + data;
    if (is_complete_scancode(SCAN_CODE)) {
        response_scancode(SCAN_CODE);
        SCAN_CODE = 0;
    }
    return;
}
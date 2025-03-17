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

int index_of(uint8_t array[], int length, uint8_t data) {
    for (int i = 0; i < length; i++) {
        if (array[i] == data) {
            return i;
        }
    }
    return -1;
}

// there are less than 256 key events, so convert multi bytes scan codes to
// single byte. Single byte scan code remains unchanged.
uint8_t to_event_id(uint64_t code) {
    if (code <= 0xff) {
        return code;
    }

    // convert 0xE0,0xxx to 0x59-0x80
    uint8_t scancodes_part1[] = {
        0x10, 0x19, 0x1C, 0x1D, 0x20, 0x21, 0x22, 0x24, 0x2E, 0x30,
        0x32, 0x35, 0x38, 0x47, 0x48, 0x49, 0x4B, 0x4D, 0x4F, 0x50,
        0x51, 0x52, 0x53, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x63, 0x65,
        0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x90, 0x99};
    uint8_t scancodes_part2[] = {0x9c, 0x9D, 0xA0, 0xA1, 0xA2, 0xA4, 0xAE, 0xB0,
                                 0xB2, 0xB5, 0xB8, 0xC7, 0xC8, 0xC9, 0xCB, 0xCD,
                                 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xDB, 0xDC, 0xDD,
                                 0xDE, 0xDF, 0xE3, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
                                 0xEA, 0xEB, 0xEC, 0xE0, 0xED};
    if (code <= 0xFFFF) {
        if (code <= 0xE09C) {
            uint8_t low_byte = code & 0xff;
            uint8_t event_id = 0x59 + index_of(scancodes_part1, 40, low_byte);
            if (in_range(event_id, 0x59, 0x80)) {
                return event_id;
            } else {
                fatal("invalid scan_code 0x%x", low_byte);
            }
        } else {
            uint8_t low_byte = code & 0xff;
            uint8_t event_id = 0xD9 + index_of(scancodes_part2, 37, low_byte);
            if (in_range(event_id, 0xD9, 0xFC)) {
                return event_id;
            } else {
                fatal("invalid scan_code 0x%x", low_byte);
            }
        }
    } else if (code <= 0xFFFFFFFF) {
        if (code == 0xe02ae037) {
            return 0xFD;
        }
        if (code == 0xe0b7e0aa) {
            return 0xFE;
        }
        uint32_t low_part = code & 0xFFFFFFFF;
    } else {
        // six bytes
        if (code == 0xE11D45E19DC5) {
            return 0xff;
        }
    }
    // invalid scan code
    uint32_t high_word = code >> 32;
    uint32_t low_word = code & 0xFFFFFFFF;
    fatal("invalid scan_code 0x%x%x", high_word, low_word);
}

bool is_valid_char(uint8_t key_id) {
    // 1234567890
    // 0x2 ~ 0xB
    if (in_range(key_id, 0x2, 0xb)) {
        return true;
    }

    // QWERTYUIOP
    // 0x10 ~ 0x19
    if (in_range(key_id, 0x10, 0x19)) {
        return true;
    }

    // ASDFGHJKL
    // 0x1e ~ 0x26
    if (in_range(key_id, 0x1e, 0x26)) {
        return true;
    }

    // ZXCVBNM
    // 0x2c ~ 0x32
    if (in_range(key_id, 0x2c, 0x32)) {
        return true;
    }

    return false;
}

bool is_shift_pressed() {
    return Key_pressed_table[0x2a] || Key_pressed_table[0x36];
}

bool is_alpha(char c) {
    return in_range(c, 'a', 'z') || in_range(c, 'A', 'Z');
}

char to_lower(char c) {
    if (is_alpha(c)) {
        if (in_range(c, 'A', 'Z')) {
            return c - ('A' - 'a');
        }
    }
    return c;
}

char to_char(KeyEvent event) {
    char *keyname = Key_table[event.key_id].name;

    if (!is_shift_pressed()) {
        return to_lower(*keyname);
    } else {
        return *keyname;
    }
}

bool Key_pressed_table[256];
bool is_key_pressed_table_initialized = false;

void response_scancode(uint64_t code) {
    uint32_t high_four_bytes = code >> 32;
    uint32_t low_four_bytes = code;
    // debug("scan_code: 0x%x%x", high_four_bytes, low_four_bytes);

    uint8_t keyevent_id = to_event_id(code);
    // debug("event_id: 0x%x %d", keyevent_id, keyevent_id);

    KeyEvent event = Keyevent_table[keyevent_id];
    printf("Keyevent: 0x%x 0x%x [", event.event_id, event.key_id);
    printf(event.keyname);
    printf("] is_pressed: %d\n", event.is_pressed);

    if (!is_key_pressed_table_initialized) {
        for (int i = 0; i < 256; i++) {
            Key_pressed_table[i] = false;
        }
        is_key_pressed_table_initialized = true;
    }

    Key_pressed_table[event.key_id] = event.is_pressed;

    // chars
    if (gets_enabled) {
        if (is_valid_char(event.key_id) && event.is_pressed) {
            gets_buffer[gets_already_count] = to_char(event);
            gets_already_count++;
            if (gets_already_count == gets_count) {
                gets_enabled = false;
                // TODO: return to user process
            }
        }
    }
}

uint64_t SCAN_CODE = 0;
void ps2_keyboard_interrupt() {
    uint32_t data = inb(KEYBOARD_DATA_PORT);
    outb(PIC1_COMMAND, EOI_CMD); // send EOI to PIC interrupt controller

    SCAN_CODE = (SCAN_CODE << 8) + data;
    if (is_complete_scancode(SCAN_CODE)) {
        response_scancode(SCAN_CODE);
        SCAN_CODE = 0;
    }
    return;
}
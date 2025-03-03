#include "bootloader32.h"
#include "kernel.h"

uint8_t scan_code_table[] = {0,
                             0, // escape
                             '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                             0,   0,   0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y',
                             'U', 'I', 'O', 'P', 0,   0,   0,   0,   'A', 'S',
                             'D', 'F', 'G', 'H', 'J', 'K', 'L', 0,   0,   0,
                             0,   0,   'Z', 'X', 'C', 'V', 'B', 'N', 'M'};

void ps2_keyboard_interrupt() {
    uint32_t scan_code = inb(0x60);
    outb(0x20, 0x20); // send Ack to keyboard

    if (scan_code == 0xE0 || scan_code == 0xe1) {
        // 2 bytes scan code
        debug("Reading multi bytes scan code");
        scan_code = (scan_code << 8) + inb(0x60);

        // 4 bytes scan code
        if (scan_code == 0xe02a || scan_code == 0xe0b7 || scan_code == 0xe11d) {

            scan_code = (scan_code << 8) + inb(0x60);
            scan_code = (scan_code << 8) + inb(0x60);
        }

        // 6 bytes
        if (scan_code == 0xe11d45e1) {
            uint16_t last_byte = inb(0x60);
            last_byte = (last_byte << 8) + inb(0x60);
            if (last_byte == 0x9dc5) {
                debug("pause pressed");
            }
        }
        debug("0x%x", scan_code);
        return;
    }

    bool is_pressed = true; // false for released
    if (scan_code > 0x80) {
        is_pressed = false;
        scan_code -= 0x80;
    }

    if (scan_code > 0x32) {
        debug("Key not recognized yet. 0x%x", scan_code);
        return;
    }
    if (scan_code_table[scan_code] == 0) {
        debug("Key not recognized yet. 0x%x", scan_code);
        return;
    }
    char c = scan_code_table[scan_code];
    if (is_pressed) {
        printf("key pressed 0x%x ", scan_code);
        print_c(c);
        printf("\n");
    } else {
        printf("key released 0x%x ", scan_code);
        print_c(c);
        printf("\n");
    }
}
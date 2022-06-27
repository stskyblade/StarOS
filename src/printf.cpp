#include "kernel.h"
#include <stdarg.h>

bool PRINT_INITIALIZED = false;
const char hexStr[20] = "0123456789abcdef";

void putc(char c) {
    if (!PRINT_INITIALIZED) {
        init_serial();
        terminal_initialize();
        PRINT_INITIALIZED = true;
    }

    terminal_putchar(c);
    write_serial(c);
}

// Write C-string to both vga & serial port, automatically initialized
void print(const char *s) {
    while (*s != '\0') {
        putc(*s);
        s++;
    }
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        uchar c = *fmt;
        if (c == '%') {
            fmt++;
            c = *fmt;

            switch (c) {
            case 'x':
                // unsigned hex
                c = va_arg(args, int) & 0xff;
                int left = c / 16;
                int right = c % 16;

                putc(hexStr[left]);
                putc(hexStr[right]);
                break;
            }

        } else {
            putc(c);
        }

        fmt++;
    }

    va_end(args);
}

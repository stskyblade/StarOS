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

    // terminal_putchar(c); this will cause interrupt 13
    write_serial(c);
}

// Write C-string to both vga & serial port, automatically initialized
void print(const char *s) {
    while (*s != '\0') {
        putc(*s);
        s++;
    }
}

void print_int(int n) {
    if (n == 0) {
        putc('0');
        return;
    }

    char s[20];
    int i = 0;

    while (n) {
        int tmp = n % 10;
        n = n / 10;
        s[i] = '0' + tmp;
        i++;
    }
    i--;

    while (i >= 0) {
        putc(s[i]);
        i--;
    }
}

void print_hex(int n) {
    print("0x");
    if (n == 0) {
        putc('0');
        return;
    }

    char s[20];
    int i = 0;

    while (n) {
        int tmp = n % 16;
        n = n / 16;
        s[i] = hexStr[tmp];
        i++;
    }
    i--;

    while (i >= 0) {
        putc(s[i]);
        i--;
    }
}

void print_binary(int n) {
    print("0b ");
    if (n == 0) {
        putc('0');
        return;
    }

    char s[100];
    int i = 0;
    int index = 0;

    while (n) {
        int tmp = n % 2;
        n = n / 2;
        s[i] = '0' + tmp;
        i++;

        if (index % 4 == 3) {
            s[i] = ' ';
            i++;
        }
        index++;
    }
    i--;

    while (i >= 0) {
        putc(s[i]);
        i--;
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
            case 'x': {
                // unsigned hex, one bit only
                c = va_arg(args, int) & 0xff;
                int left = c / 16;
                int right = c % 16;

                putc(hexStr[left]);
                putc(hexStr[right]);
                break;
            }

            case 'p': {
                uint value = va_arg(args, int);
                print_hex(value);
                break;
            }

            case 'd': {
                int num = va_arg(args, int);
                print_int(num);
                break;
            }

            case 'b': {
                int num = va_arg(args, int);
                print_binary(num);
                break;
            }
            }

        } else {
            putc(c);
        }

        fmt++;
    }

    va_end(args);
}

void panic(const char *c) {
    printf(c);
    while (true) {
        ;
    }
}

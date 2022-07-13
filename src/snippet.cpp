#include "types.h"

void memcopy(uchar *dest, uchar *source, size_t count) {
    for (size_t i = 0; i < count; i++) {
        dest[i] = source[i];
    }
}

bool strcmp(const char *a, const char *b) {
    while (*a == *b) {
        if (*a == '\0') {
            return true;
        }
        a++;
        b++;
    }
    return false;
}

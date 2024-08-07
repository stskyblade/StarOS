#include <stdint.h>

extern "C" {

    // never return
void bootloader32_start () {
    uint16_t a = 3;
    uint16_t b = 4;
    uint16_t c = a + b;

    uint32_t x = 3;
    uint32_t y = 4;
    uint32_t z = x + y;

    while(1){
        ;
    }
}

}
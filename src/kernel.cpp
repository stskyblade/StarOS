#include "bootloader32.h"

void interrupt_handler() {
    panic("This is interrupt_handler\n");
}

// Page 157. Figure 9-3
struct Gate_Descriptor {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t type : 5;
    uint8_t dpl : 2;
    uint8_t p : 1; // 1 for present, 0 for not-present exception
    uint16_t offset_high;
} __attribute__((packed));

const int IDT_size = 256;
Gate_Descriptor IDT[IDT_size];
uint16_t IDTR[3];

void init_interrupt_handler() {
    Gate_Descriptor desc; // interrupt gate
    uint32_t handler_pointer = (uint32_t)interrupt_handler;

    desc.offset_low = handler_pointer & 0xFFFF;
    desc.offset_high = handler_pointer >> 16;
    desc.selector = 0x0008; // points to GDT[1] in bootloader.S
    desc.reserved = 0b00000000;
    desc.type = 0b01110;
    desc.dpl = 0b00;
    desc.p = 0b1;

    for (int i = 0; i < IDT_size; i++) {
        IDT[i] = desc;
    }

    // set IDTR
    IDTR[0] = sizeof(Gate_Descriptor) * IDT_size - 1; // limit
    // TODO: is it valid to ignore 0xFFFF below?
    IDTR[1] = ((uint32_t)IDT) & 0xFFFF; // offset
    IDTR[2] = ((uint32_t)IDT) >> 16;
    __asm__ __volatile__("lidt %0\n\t"
                         :
                         : "m"(IDTR));
    __asm__ __volatile__("sti\n\t"
                         :
                         :);
}

extern "C" {
void kernel_main() {
    printf("hello,kernel\n");
    init_interrupt_handler();

    // never return
    while (1) {
        ;
    }
}
}

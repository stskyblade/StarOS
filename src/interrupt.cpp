#include "kernel.h"

struct interrupt_gate {
    short offset;
    short selector;
    short data;
    short offset2;
};

interrupt_gate IDT[256];

struct segment_descriptor {
    short limit;
    short base;
    short base2;
    short base3;
};

segment_descriptor GDT[256];

void interrupt_handler() {
    printf("this is interrupt handler");
    asm volatile("iret");
}

void init_interrupt_handler() {
    printf("init interrupt...\n");
    short offset = ((int)&interrupt_handler) % 16;
    short base = ((int)&interrupt_handler) / 16;
    short selector = 1;

    segment_descriptor d;
    d.limit = 0xff;
    d.base = base;
    d.base2 = 0x0000;
    d.base3 = 0x0000;
    GDT[selector] = d; // 0 is reserved

    // load GDTR
    asm volatile("LGDT (%0)" ::"d"(GDT));

    // interrupt gate
    interrupt_gate g;
    g.offset = offset;
    g.selector = selector;
    g.data = 0x0f00;
    g.offset2 = 0;
    IDT[0] = g;
    for (int i = 0; i < 256; i++) {
        IDT[i] = g;
        GDT[i] = d;
    }

    // load IDT
    short idtr[3];
    idtr[0] = 0xff;
    idtr[1] = (uint)IDT;
    idtr[2] = (uint)IDT >> 16;

    asm volatile("lidt (%0)" ::"r"(idtr));

    printf("before sti\n");
    asm volatile("sti");
    printf("before int 0\n");
    asm volatile("int $0");
    printf("after int 0\n");
}

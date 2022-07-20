#include "kernel.h"

struct interrupt_gate {
    short offset;
    short selector;
    short data;
    short offset2;
};

struct gate_descriptor {
    uint off_15_0 : 16;
    uint cs : 16;
    uint reserved : 8;
    uint type : 5;
    uint dpl : 2;
    uint p : 1;
    uint off_31_16 : 16;
};

gate_descriptor IDT[256];

struct segment_descriptor {
    short limit;
    short base;
    short base2;
    short base3;
};

struct seg_descriptor {
    uint limit_15_0 : 16;
    uint base_15_0 : 16;
    uint base_23_16 : 8;
    uint type : 4;       // Segment type (see STS_ constants)
    uint s : 1;          // 0 = system, 1 = application
    uint dpl : 2;        // Descriptor Privilege Level
    uint p : 1;          // Present
    uint lim_19_16 : 4;  // High bits of segment limit
    uint avl : 1;        // Unused (available for software use)
    uint rsv1 : 1;       // Reserved
    uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
    uint g : 1;          // Granularity: limit scaled by 4K when set
    uint base_31_24 : 8; // High bits of segment base address
};

seg_descriptor GDT[256];
extern uint32_t vectors[256];

// by trapasm.S
struct trap_frame {
    // passed by pushal
    uint edi;
    uint esi;
    uint ebp;
    uint oesp; // useless & ignored
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;

    ushort gs;
    ushort padding1;
    ushort fs;
    ushort padding2;
    ushort es;
    ushort padding3;
    ushort ds;
    ushort padding4;
    uint trapno;

    // below here defined by x86 hardware
    uint err;
    uint eip;
    ushort cs;
    ushort padding5;
    uint eflags;

    // below here only when crossing rings, such as from user to kernel
    uint esp;
    ushort ss;
    ushort padding6;
};

extern "C" {
void interrupt_handler(trap_frame *tf) {
    printf("this is interrupt handler: %d\n", tf->trapno);
    asm volatile("iret");
}
}

void init_interrupt_handler() {
    printf("init interrupt...\n");
    //    short offset = ((int)&interrupt_handler) % 16;
    // short base = ((int)&interrupt_handler) / 16;
    int handler_func = (int)&interrupt_handler;
    short selector = 1;

    seg_descriptor d;
    d.limit_15_0 = 0xffff;
    d.base_15_0 = 0;
    d.base_23_16 = 0;
    d.type = 0x8 | 0x2;
    d.s = 1;
    d.dpl = 0;
    d.p = 1;
    d.lim_19_16 = 0xf;
    d.avl = 0;
    d.rsv1 = 0;
    d.db = 1;
    d.g = 1;
    d.base_31_24 = 0;
    GDT[selector] = d; // 0 is reserved

    // load GDTR
    short gdtr[3];
    gdtr[0] = 0xff;
    gdtr[1] = (uint)GDT;
    gdtr[2] = (uint)GDT >> 16;

    asm volatile("LGDT (%0)" ::"r"(gdtr));

    // interrupt gate
    gate_descriptor g;
    g.off_15_0 = handler_func & 0xffff; // directely to handler_func
    g.cs = selector << 3;
    g.reserved = 0;
    g.type = 0xe; // interrupt gate
    g.dpl = 0;
    g.p = 1;
    g.off_31_16 = handler_func >> 16;

    IDT[0] = g;
    for (int i = 0; i < 256; i++) {
        IDT[i] = g;
        uint32_t func_addr;
        func_addr = vectors[i];
        g.off_15_0 = func_addr & 0xffff; // auto pass interrupt id to handler_func
        g.off_31_16 = func_addr >> 16;

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

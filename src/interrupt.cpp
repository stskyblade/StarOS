#include "kernel.h"

#define SEG(type, base, lim, dpl)                             \
    (struct seg_descriptor) {                                 \
        ((lim) >> 12) & 0xffff, (uint)(base)&0xffff,          \
            ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,     \
            (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 \
    }

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
seg_descriptor LDT[256];
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

    // below here defined by x86 hardware, page 159
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
    printf("=========== interrupt handler: %d========\n", tf->trapno);
    switch (tf->trapno) {
    case 13:
        printf(".\n");
        break;
    case 8: {
        // panic("trap 8");
        break;
    }
    default:
        break;
    }

    // printf("==================interrupt handler\n");
    // return;
    // while (true) {
    //     // printf(".\n");
    //     // return;
    //     ;
    // }

    // asm volatile("cli");
    printf("=================interrupt_handler===================\n");
    printf("this is interrupt handler: %d\n", tf->trapno);
    printf("err code: %d\n", tf->err);
    printf("cs: eip  %d: %d\n", tf->cs, tf->eip);
    uint selector_index = (tf->err & 0xfff8) >> 3;
    uint bits = tf->err & 0b111; // page 162
    printf("selector_index: %d\n", selector_index);
    printf("bits: %d \n", bits);

    // from IDT
    if (bits & 0x2) {
        gate_descriptor g = IDT[selector_index];
        selector_index = g.cs >> 3;
        seg_descriptor s = GDT[selector_index];
        uint present_bit = s.p;
        printf("selector_index: %d\n", selector_index);
        printf("present_bit: %d\n", present_bit);
    }

    printf("=================interrupt_handler finish===================\n");
    // asm volatile("sti");
    printf("trap:\n");
    printf("edi: %p %d\n", tf->edi, tf->edi);
    printf("esi: %p %d\n", tf->esi, tf->esi);
    printf("ebp: %p %d\n", tf->ebp, tf->ebp);
    printf("oesp: %p %d\n", tf->oesp, tf->oesp);
    printf("ebx: %p %d\n", tf->ebx, tf->ebx);
    printf("edx: %p %d\n", tf->edx, tf->edx);
    printf("ecx: %p %d\n", tf->ecx, tf->ecx);
    printf("eax: %p %d\n", tf->eax, tf->eax);
    printf("-------------------\n");
    printf("gs: %p %d\n", tf->gs, tf->gs);
    printf("fs: %p %d\n", tf->fs, tf->fs);
    printf("es: %p %d\n", tf->es, tf->es);
    printf("ds: %p %d\n", tf->ds, tf->ds);
    printf("trapno: %p %d\n", tf->trapno, tf->trapno);
    printf("-------------------\n");
    printf("err: %p %d\n", tf->err, tf->err);
    printf("eip: %p %d\n", tf->eip, tf->eip);
    printf("cs: %p %d\n", tf->cs, tf->cs);
    printf("eflags: %p %d %b\n", tf->eflags, tf->eflags, tf->eflags);
    printf("-------------------\n");
    printf("esp: %p %d\n", tf->esp, tf->esp);
    printf("ss: %p %d\n", tf->ss, tf->ss);
    printf("-------------------\n");
    //    panic("trap in interrupt_handler\n");

    switch (tf->trapno) {
    case 32:
        lapiceoi();
        // panic("trap\n");
        break;

    default:
        break;
    }
}
}

seg_descriptor pack_segment_desc(uint base, uint limit, uint access_byte, uint flags) {
    seg_descriptor d;
    d.limit_15_0 = limit & 0xffff;
    d.base_15_0 = base & 0xffff;
    d.base_23_16 = (base >> 16) & 0xff;
    d.type = access_byte & 0xf;
    d.s = (access_byte >> 4) & 0x1;
    d.dpl = (access_byte >> 5) & 0x3;
    d.p = (access_byte >> 7) & 0x1;
    d.lim_19_16 = (limit >> 16) & 0xf;
    d.avl = flags & 0x1;
    d.rsv1 = (flags >> 1) & 0x1;
    d.db = (flags >> 2) & 0x1;
    d.g = (flags >> 3) & 0x1;
    d.base_31_24 = (base >> 24) & 0xff;
    return d;
}

gate_descriptor pack_gate_desc(int index) {
    gate_descriptor g;
    g.cs = 1 << 3; // kcode
    g.reserved = 0;
    g.type = 0xe; // interrupt gate
    g.dpl = 0;
    g.p = 1;

    uint32_t func_addr;
    func_addr = vectors[index];
    g.off_15_0 = func_addr & 0xffff; // auto pass interrupt id to handler_func
    g.off_31_16 = func_addr >> 16;

    return g;
}

void init_GDT() {
    GDT[0] = pack_segment_desc(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff); // null descriptor
    GDT[1] = pack_segment_desc(0, 0xfffff, 0x9a, 0xc);                          // kernel code
    GDT[2] = pack_segment_desc(0, 0xfffff, 0x92, 0xc);                          // kernel data
    GDT[3] = pack_segment_desc(0, 0xfffff, 0xfa, 0xc);                          // user code
    GDT[4] = pack_segment_desc(0, 0xfffff, 0xf2, 0xc);                          // user data
    GDT[5] = pack_segment_desc(0, 0xfffff, 0x89, 0x0);                          // tss

    // load GDTR
    short gdtr[3];
    gdtr[0] = sizeof(GDT) - 1;
    gdtr[1] = (uint)GDT;
    gdtr[2] = (uint)GDT >> 16;

    asm volatile("LGDT (%0)" ::"r"(gdtr));
}

void init_interrupt_handler() {
    printf("\ninit interrupt...\n");
    init_GDT();

    for (int i = 0; i < 256; i++) {
        IDT[i] = pack_gate_desc(i);
    }

    // load IDT
    short idtr[3];
    idtr[0] = sizeof(IDT) - 1;
    idtr[1] = (uint)IDT;
    idtr[2] = (uint)IDT >> 16;

    asm volatile("lidt (%0)" ::"r"(idtr));

    printf("before sti\n");
    asm volatile("sti");

    // for (int i = 0; i < 3; i++) {
    //     asm volatile("int $64");
    //     printf("after int\n");
    // }

    // while (true) {
    //     printf(".");
    //     ;
    // }
}

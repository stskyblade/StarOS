#include "kernel.h"

// Advanced Programmable Interrupt Controller (APIC)
// config, timer clock

const uint APIC_BASE_ADDR = 0x1b;

// return true if APIC is supported by the CPU
bool check_apic() {
    uint edx = 0;
    asm volatile("mov $0x1, %%eax\n\t"
                 "cpuid\n\t"
                 "mov %%edx, %0"
                 : "=rm"(edx));
    return edx & (1 << 9);
}

// Model Specific Register read
void cpuGetMSR(uint32_t msr, uint32_t &lo, uint32_t &hi) {
    asm volatile("rdmsr"
                 : "=a"(lo), "=d"(hi)
                 : "c"(msr));
}

// Model Specific Register write
void cpuSetMSR(uint32_t msr, uint32_t &lo, uint32_t &hi) {
    asm volatile("wrmsr"
                 :
                 : "a"(lo), "d"(hi), "c"(msr));
}

// base address of memory which APIC memory-mapped to
uint *cpu_get_apic_base() {
    uint eax, edx;
    cpuGetMSR(APIC_BASE_ADDR, eax, edx);
    uint *p = (uint *)(eax & 0xfffff000);
    return p;
}

// base address of memory which APIC memory-mapped to
void cpu_set_apic_base(uint *apic_addr) {
    uint edx = 0;
    uint eax = ((uint)apic_addr & 0xfffff0000) | 0x800;

    cpuSetMSR(APIC_BASE_ADDR, eax, edx);
}

uint *apic_base = 0; // base address of memory which APIC memory-mapped to

// write value to local APIC related address, which has index offset to apic_base
static void lapicw(uint index, uint value) {
    if (!apic_base) {
        panic("apic base invalid...");
    }

    apic_base[index] = value;
    apic_base[ID];
}

// Acknowledge interrupt. If not use this function, APIC will not generate new interrupt
void lapiceoi(void) {
    if (apic_base)
        lapicw(EOI, 0);
}

// enable APIC, enable timer
void init_apic() {
    // https://wiki.osdev.org/APIC#Local_APIC_configuration
    printf("init apic\n");

    if (!check_apic()) {
        return;
    } else {
        printf("apic supported\n");
    }

    apic_base = cpu_get_apic_base();
    cpu_set_apic_base(apic_base);

    // set SIVR
    lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));

    lapicw(TDCR, X1); // divide configuration register
    lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
    lapicw(TICR, 10000000); // initial count

    // TODO:

    // // Disable logical interrupt lines.
    // lapicw(LINT0, MASKED);
    // lapicw(LINT1, MASKED);

    // // Disable performance counter overflow interrupts
    // // on machines that provide that interrupt entry.
    // if (((apic_base[VER] >> 16) & 0xFF) >= 4)
    //     lapicw(PCINT, MASKED);

    // // Map error interrupt to IRQ_ERROR.
    // lapicw(ERROR, T_IRQ0 + IRQ_ERROR);

    // // Clear error status register (requires back-to-back writes).
    // lapicw(ESR, 0);
    // lapicw(ESR, 0);

    // // Ack any outstanding interrupts.
    // lapicw(EOI, 0);

    // // Send an Init Level De-Assert to synchronise arbitration ID's.
    // lapicw(ICRHI, 0);
    // lapicw(ICRLO, BCAST | INIT | LEVEL);
    // while (apic_base[ICRLO] & DELIVS)
    //     ;

    lapicw(TPR, 0);

    printf("apic timer enabled....\n");

    int max = 50;
    while (max--) {
        printf("ticr: %p %d\n", apic_base[TCCR], apic_base[TCCR]);
    }
    panic("trap");
}

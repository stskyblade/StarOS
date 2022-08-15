#include "kernel.h"

// x86 trap and interrupt constants.

// Processor-defined:
#define T_DIVIDE 0 // divide error
#define T_DEBUG 1  // debug exception
#define T_NMI 2    // non-maskable interrupt
#define T_BRKPT 3  // breakpoint
#define T_OFLOW 4  // overflow
#define T_BOUND 5  // bounds check
#define T_ILLOP 6  // illegal opcode
#define T_DEVICE 7 // device not available
#define T_DBLFLT 8 // double fault
// #define T_COPROC      9      // reserved (not used since 486)
#define T_TSS 10   // invalid task switch segment
#define T_SEGNP 11 // segment not present
#define T_STACK 12 // stack exception
#define T_GPFLT 13 // general protection fault
#define T_PGFLT 14 // page fault
// #define T_RES        15      // reserved
#define T_FPERR 16   // floating point error
#define T_ALIGN 17   // aligment check
#define T_MCHK 18    // machine check
#define T_SIMDERR 19 // SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL 64  // system call
#define T_DEFAULT 500 // catchall

#define T_IRQ0 32 // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

// Local APIC registers, divided by 4 for use as uint[] indices.
#define ID (0x0020 / 4)    // ID
#define VER (0x0030 / 4)   // Version
#define TPR (0x0080 / 4)   // Task Priority
#define EOI (0x00B0 / 4)   // EOI
#define SVR (0x00F0 / 4)   // Spurious Interrupt Vector
#define ENABLE 0x00000100  // Unit Enable
#define ESR (0x0280 / 4)   // Error Status
#define ICRLO (0x0300 / 4) // Interrupt Command
#define INIT 0x00000500    // INIT/RESET
#define STARTUP 0x00000600 // Startup IPI
#define DELIVS 0x00001000  // Delivery status
#define ASSERT 0x00004000  // Assert interrupt (vs deassert)
#define DEASSERT 0x00000000
#define LEVEL 0x00008000 // Level triggered
#define BCAST 0x00080000 // Send to all APICs, including self.
#define BUSY 0x00001000
#define FIXED 0x00000000
#define ICRHI (0x0310 / 4)  // Interrupt Command [63:32]
#define TIMER (0x0320 / 4)  // Local Vector Table 0 (TIMER)
#define X1 0x0000000B       // divide counts by 1
#define PERIODIC 0x00020000 // Periodic
#define PCINT (0x0340 / 4)  // Performance Counter LVT
#define LINT0 (0x0350 / 4)  // Local Vector Table 1 (LINT0)
#define LINT1 (0x0360 / 4)  // Local Vector Table 2 (LINT1)
#define ERROR (0x0370 / 4)  // Local Vector Table 3 (ERROR)
#define MASKED 0x00010000   // Interrupt masked
#define TICR (0x0380 / 4)   // Timer Initial Count
#define TCCR (0x0390 / 4)   // Timer Current Count
#define TDCR (0x03E0 / 4)   // Timer Divide Configuration

const uint APIC_BASE_ADDR = 0x1b;

bool check_apic() {
    uint edx = 0;
    asm volatile("mov $0x1, %%eax\n\t"
                 "cpuid\n\t"
                 "mov %%edx, %0"
                 : "=rm"(edx));
    return edx & (1 << 9);
}

void cpuGetMSR(uint32_t msr, uint32_t &lo, uint32_t &hi) {
    asm volatile("rdmsr"
                 : "=a"(lo), "=d"(hi)
                 : "c"(msr));
}

void cpuSetMSR(uint32_t msr, uint32_t &lo, uint32_t &hi) {
    asm volatile("wrmsr"
                 :
                 : "a"(lo), "d"(hi), "c"(msr));
}

uint *cpu_get_apic_base() {
    uint eax, edx;
    cpuGetMSR(APIC_BASE_ADDR, eax, edx);
    uint *p = (uint *)(eax & 0xfffff000);
    return p;
}

void cpu_set_apic_base(uint *apic) {
    uint edx = 0;
    uint eax = ((uint)apic & 0xfffff0000) | 0x800;

    cpuSetMSR(APIC_BASE_ADDR, eax, edx);
}

uint *apic_base = 0;

static void
lapicw(uint index, uint value) {
    if (!apic_base) {
        panic("apic base invalid...");
    }

    apic_base[index] = value;
    apic_base[ID];
}

// Acknowledge interrupt.
void lapiceoi(void) {
    if (apic_base)
        lapicw(EOI, 0);
}

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

    lapicw(TDCR, X1);
    lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
    lapicw(TICR, 10000000);

    // Disable logical interrupt lines.
    lapicw(LINT0, MASKED);
    lapicw(LINT1, MASKED);

    // Disable performance counter overflow interrupts
    // on machines that provide that interrupt entry.
    if (((apic_base[VER] >> 16) & 0xFF) >= 4)
        lapicw(PCINT, MASKED);

    // Map error interrupt to IRQ_ERROR.
    lapicw(ERROR, T_IRQ0 + IRQ_ERROR);

    // Clear error status register (requires back-to-back writes).
    lapicw(ESR, 0);
    lapicw(ESR, 0);

    // Ack any outstanding interrupts.
    lapicw(EOI, 0);

    // Send an Init Level De-Assert to synchronise arbitration ID's.
    lapicw(ICRHI, 0);
    lapicw(ICRLO, BCAST | INIT | LEVEL);
    while (apic_base[ICRLO] & DELIVS)
        ;

    lapicw(TPR, 0);

    printf("apic timer enabled....\n");

    int max = 50;
    while (max--) {
        printf("ticr: %p %d\n", apic_base[TCCR], apic_base[TCCR]);
    }
    panic("trap");
}

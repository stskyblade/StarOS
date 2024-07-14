#include "types.h"

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
// Why divided by 4?
// each register below is 32 bits, takes up 4 bytes. Their offset is for 1 byte.
// we use uint[], if we want access n th byte of base addr, base[n/4] is required
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

int init_serial();
void write_serial(char a);
void test_read_serial();

void terminal_initialize(void);
void terminal_putchar(char c);

void print(const char *c);
void printf(const char *fmt, ...);
void panic(const char *c);

bool read_sector(uint addr, size_t sector_count, uchar *buffer);

void list_dir(const char *dir);
dir_entry_t *read_dir(dir_t &d);
FILE *fopen(const char *pathname);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f);

void memcopy(uchar *dest, uchar *source, size_t count);
bool strcmp(const char *a, const char *b);

void init_interrupt_handler();
void init_apic();
void lapiceoi();

static inline void outb(ushort port, uchar data) {
    asm volatile("out %0,%1"
                 :
                 : "a"(data), "d"(port));
}

static inline void out32(ushort port, uint data) {
    asm volatile("out %0,%1"
                 :
                 : "a"(data), "d"(port));
}

static inline uchar inb(ushort port) {
    uchar data;

    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline ushort inw(ushort port) {
    ushort data;

    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline uint in32(ushort port) {
    uint data;

    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

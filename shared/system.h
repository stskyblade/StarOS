#pragma once
#include <stdint.h>
typedef uint32_t size_t;
// Interrupt number used by system call
constexpr char SYSCALL_INT_ID = 32;
constexpr char PIC1_BASE_ID = 0x30;
constexpr char PIC2_BASE_ID = 0x38;

// System Call Numbers, pass by EAX
// Only use six registers: EAX, EBX, ECX, EDX, ESI, EDI
// EAX for syscall number
// Other small arguments are put into these registers from left to right, in
// order
// Return value is put in EAX
constexpr int SYSCALL_HELLOWORLD = 0;
constexpr int SYSCALL_ADD = 1;
constexpr int SYSCALL_STRUCT = 2;
constexpr int SYSCALL_PRINTF = 3;
constexpr int SYSCALL_GETS = 4;
constexpr int SYSCALL_SLEEP = 5;

// IRQ numbers
constexpr int IRQ_TIMER = 0;
constexpr int IRQ_KEYBOARD = 1;

// Struct for test only
struct TestPack {
    bool a1;
    char a2;
    short a3;
    int a4;
    double a5;
    int a6;
    short a7;
    char a8;
    bool a9;
    short a10;
};

constexpr int ORIGIN_FREQUENCY = 1193182; // 1.19 mHz
constexpr int FREQUENCY_WANTED = 1000;
constexpr int64_t TICKS_PER_SECOND = FREQUENCY_WANTED;
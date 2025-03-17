#include "bootloader32.h"
#include "kernel.h"

// copy from https://wiki.osdev.org/8259_PIC#Code_Examples
// https://wiki.osdev.org/Programmable_Interval_Timer#Mode_2_%E2%80%93_Rate_Generator

// config Timer
void init_PIT() {
    char channel = 0b00;     // channel 0
    char access_mode = 0b11; // lobyte/hibyte
    char operating_mode = 0b011;
    char BCD_mode = 0b0;
    char cmd =
        (channel << 6) + (access_mode << 4) + (operating_mode << 1) + BCD_mode;
    outb(PIT_COMMAND_PORT, cmd);

    short count = 0;                         // 65536, 18.2Hz
    outb(CHANNEL_0_DATA_PORT, count & 0xff); // low byte
    outb(CHANNEL_0_DATA_PORT, count >> 8);   // high byte
}
// initialize PIC(Program Interrupt Controller)
void init_PIC() {
    debug("Start init PIC...");
    outb(PIC1_COMMAND,
         ICW1_INIT |
             ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, PIC1_BASE_ID); // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, PIC2_BASE_ID); // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at
                        // IRQ2 (0000 0100)
    io_wait();
    outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    outb(PIC1_DATA,
         ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    init_PIT();

    // Unmask both PICs.
    // Only accept Keyboard interrupt at this time
    // https://wiki.osdev.org/Interrupts#Types_of_Interrupts
    outb(PIC1_DATA, 0b11111100); // keyboard interrupt and timer interrupt
    outb(PIC2_DATA, 0xff);
    debug("Finished init PIC.");
}

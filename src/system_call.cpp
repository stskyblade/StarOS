// This file implements all system calls in kernel.

#include "bootloader32.h"
#include "kernel.h"

void system_entry(int syscall_id, TrapFrame *tf) {
    debug("Start of system_entry");
    switch (syscall_id) {
    case SYSCALL_HELLOWORLD:
        info("hello,world from system call in kernel");
        break;
    case SYSCALL_ADD: {
        int result = tf->ebx + tf->ecx;
        debug("add: %d + %d = %d", tf->ebx, tf->ecx, result);
        tf->eax = result;
    } break;
    default:
        printf("%d ");
        fatal("syscall id not supported");
        break;
    }
    return;
}

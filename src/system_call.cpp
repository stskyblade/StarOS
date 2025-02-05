// This file implements all system calls in kernel.

#include "bootloader32.h"
#include "kernel.h"

void system_entry(int syscall_id, TrapFrame *tf) {
    switch (syscall_id) {
    case SYSCALL_HELLOWORLD:
        info("hello,world from system call in kernel\n");
        break;
    default:
        break;
    }
    return;
}

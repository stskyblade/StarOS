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
    case SYSCALL_STRUCT: {
        debug("syscall struct");
        uint32_t pointer = tf->ebx;
        uint32_t size = tf->ecx;
        debug("pointer of struct: 0x%x", pointer);
        // struct pointed by pointer is not mapped in kernel.
        // Find that process, add memory mapping.
        // pointer remains the same.
        copy_process_mapping((char *)pointer, size);
        // pointer is valid now

        TestPack *p = (TestPack *)pointer;
        int data = p->a1;
        debug("a1: %d", data);
        data = p->a2;
        debug("a2: %d", data);
        data = p->a3;
        debug("a3: %d", data);
        data = p->a4;
        debug("a4: %d", data);
        data = p->a5;
        debug("a5: %d", data);
        data = p->a6;
        debug("a6: %d", data);
        data = p->a7;
        debug("a7: %d", data);
        data = p->a8;
        debug("a8: %d", data);
        data = p->a9;
        debug("a9: %d", data);
        data = p->a10;
        debug("a10: %d", data);
    } break;
    case SYSCALL_PRINTF: {
        debug("syscall printf");
        char *p = (char *)tf->ebx;
        uint32_t size = tf->ecx;
        copy_process_mapping(p, size);
        debug("copy mapping done");
        printf(p);
    } break;
    default:
        printf("%d ");
        fatal("syscall id not supported");
        break;
    }
    return;
}

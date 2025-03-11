// This file implements all system calls in kernel.

#include "bootloader32.h"
#include "kernel.h"
#include "linked_list.h"

bool gets_enabled = false;
int gets_count = 0;         // how many bytes to wait
int gets_already_count = 0; // how many bytes have been read
char *gets_buffer;
int gets_process_id; // the id of process waiting for input

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
        // format string
        char *p = (char *)tf->ebx;
        uint32_t size = tf->ecx;
        copy_process_mapping(p, size);
        debug("copy mapping done");

        // ... arguments
        char *args = (char *)tf->edx;
        uint32_t last_arg_size = tf->esi;
        copy_process_mapping(args, 1024 * 4);
        // process stack is different from kernel stack, so can't simply use it
        // just copy 10 * 4B, should be enough
        uint32_t *first_ellipses_arg = (uint32_t *)(args + last_arg_size);
        // push last 4B first
        for (int i = 0; i < 10; i++) {
            uint32_t last_ellipses_arg = *(first_ellipses_arg + 9 - i);
            __asm__ __volatile__("pushl %0\n\t" ::"m"(last_ellipses_arg));
        }
        // ellipses args are manually pushed to stack
        __asm__ __volatile__("pushl %0\n\t" ::"m"(p));
        __asm__ __volatile__("call _Z6printfPKcz\n\t" ::);
        __asm__ __volatile__("add    $0x10,%%esp\n\t" ::);
        __asm__ __volatile__(
            "add    $0x28,%%esp\n\t" ::); // 40B for ellipses args
    } break;
    case SYSCALL_GETS: {
        // TODO
        debug("syscall gets");
        uint32_t pointer = tf->ebx;
        uint32_t size = tf->ecx;
        copy_process_mapping((char *)pointer, size);

        // save TrapFrame to Process context
        // how to find current Process?
        // current_proc and running_queue[n] doesn't point to the same memory.
        // Or running_queue just store pointer of Process rather than struct
        // Process.
        // Or current_proc points the memory inside running_queue.
        // I just need a easy way to change the content of Process.
        save_context_from_trapframe(CURRENT_PROCESS->context, tf);

        if (Current_control_flow != Process_thread) {
            fatal("Invalid source of system entry");
        }

        gets_enabled = true;
        gets_count = size;
        gets_already_count = 0;
        gets_buffer = (char *)pointer;
        gets_process_id = CURRENT_PROCESS->id;

        // TODO:
        // move to blocking queue
        running_queue.remove(CURRENT_PROCESS);
        blocking_queue.push_back(CURRENT_PROCESS);
        CURRENT_PROCESS->status = Blocking;
        // how to return from keyboard interrupt?
        restore_context_to_trapframe(Kernel_proc.context, tf);

        debug("return from system_entry");
    } break;
    default:
        printf("%d ");
        fatal("syscall id not supported");
        break;
    }
    return;
}

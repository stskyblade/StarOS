// This file implements all system calls in kernel.

#include "bootloader32.h"
#include "kernel.h"
#include "linked_list.h"

bool gets_enabled = false;
int gets_count = 0;

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

        // save TrapFrame to Process context
        // how to find current Process?
        // current_proc and running_queue[n] doesn't point to the same memory.
        // Or running_queue just store pointer of Process rather than struct
        // Process.
        // Or current_proc points the memory inside running_queue.
        // I just need a easy way to change the content of Process.
        Context &current_proc = CURRENT_PROCESS->context;
        // uint32_t edi;
        current_proc.edi = tf->edi;
        // uint32_t esi;
        current_proc.esi = tf->esi;
        // uint32_t ebp;
        current_proc.ebp = tf->ebp;
        // uint32_t esp;
        current_proc.esp = tf->old_esp;
        // uint32_t ebx;
        current_proc.ebx = tf->ebx;
        // uint32_t edx;
        current_proc.edx = tf->edx;
        // uint32_t ecx;
        current_proc.ecx = tf->ecx;
        // uint32_t eax;
        current_proc.eax = tf->eax;
        // uint32_t gs;
        current_proc.gs = tf->gs;
        // uint32_t fs;
        current_proc.fs = tf->fs;
        // uint32_t es;
        current_proc.es = tf->es;
        // uint32_t ds;
        current_proc.ds = tf->ds;

        // FIXME: need to distinguish from interrupt from kernel space or user
        // space uint32_t ss;
        current_proc.ss = tf->old_ss;
        // uint32_t cs;
        current_proc.cs = tf->old_cs;
        // uint32_t eip;
        current_proc.eip = tf->return_addr;
        // uint32_t eflags;
        current_proc.eflags = tf->old_eflags;

        if (Current_control_flow != Process_thread) {
            fatal("Invalid source of system entry");
        }

        gets_enabled = true;
        gets_count = size;

        // TODO:
        // move to blocking queue
        running_queue.remove(CURRENT_PROCESS);
        blocking_queue.push_back(CURRENT_PROCESS);
        CURRENT_PROCESS->status = Blocking;
        // how to return from keyboard interrupt?
        Context &kernel_cxt = Kernel_proc.context;
        tf->eax = kernel_cxt.eax;
        tf->ebx = kernel_cxt.ebx;
        tf->ecx = kernel_cxt.ecx;
        tf->edx = kernel_cxt.edx;
        tf->esi = kernel_cxt.esi;
        tf->edi = kernel_cxt.edi;
        // tf->esp = kernel_cxt.esp; keep it
        tf->ebp = kernel_cxt.ebp;
        tf->ds = kernel_cxt.ds;
        tf->es = kernel_cxt.es;
        tf->fs = kernel_cxt.fs;
        tf->gs = kernel_cxt.gs;
        tf->return_addr = kernel_cxt.eip;
        tf->old_cs = kernel_cxt.cs;
        tf->old_ss = kernel_cxt.ss;
        tf->old_esp = kernel_cxt.esp;
        tf->old_eflags = kernel_cxt.eflags;
        int pdbr = (int)kernel_cxt.page_directory;
        __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(pdbr));
        debug("return from system_entry");
    } break;
    default:
        printf("%d ");
        fatal("syscall id not supported");
        break;
    }
    return;
}

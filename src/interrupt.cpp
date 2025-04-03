#include "Configs.h"
#include "bootloader32.h"
#include "kernel.h"

bool return_to_process = false;

bool is_hardware_interrupt(uint32_t condition_code) {
    if (condition_code >= PIC1_BASE_ID && condition_code < (PIC1_BASE_ID + 8)) {
        return true;
    }
    if (condition_code >= PIC2_BASE_ID && condition_code < (PIC2_BASE_ID + 8)) {
        return true;
    }
    return false;
}

bool operator==(CountDownClock left, CountDownClock right) {
    return left.count_down == right.count_down && left.process == right.process;
}
void hardware_interrupt_handler(uint32_t condition_code, TrapFrame *tf) {
    // FIXME: syscall gets
    // convert interrupt vector to IRQ number
    int IRQ = -1;
    if (condition_code >= PIC1_BASE_ID && condition_code < (PIC1_BASE_ID + 8)) {
        IRQ = condition_code - PIC1_BASE_ID;
    }
    if (condition_code >= PIC2_BASE_ID && condition_code < (PIC2_BASE_ID + 8)) {
        IRQ = condition_code - PIC2_BASE_ID;
    }

    uint8_t data = 0;
    // https://wiki.osdev.org/Interrupts#Types_of_Interrupts
    switch (IRQ) {
    case IRQ_TIMER: {
        // return to kernel scheduler only when interrupted from Process thread
        trace("timer interrupt");
        Count_down--;

        // Check all waiting sleep process, move to ready queue if possible
        // auto *prev = &waiting_sleep_queue.head;
        // auto *cur = prev->next;
        // while (cur) {
        //     cur->data.count_down--;
        //     if (cur->data.count_down == 0) {
        //         // move to ready queue
        //         Process *p = cur->data.process;
        //         blocking_queue.remove(p);
        //         ready_queue.push_back(p);
        //         p->status = Ready;

        //         // remove cur node
        //         waiting_sleep_queue.remove(cur->data);
        //         // prev->next = cur->next;
        //         // waiting_sleep_queue.tail = prev; // TODO
        //         // free(cur);
        //         cur = prev->next;
        //         continue;
        //     }
        //     prev = cur;
        //     cur = cur->next;
        // }
        waiting_sleep_queue.for_each([](CountDownClock &clock) {
            clock.count_down--;
            if (clock.count_down == 0) {
                Process *p = clock.process;
                blocking_queue.remove(p);
                ready_queue.push_back(p);
                p->status = Ready;
            }
        });
        // if count_down == 0, remove
        waiting_sleep_queue.filter(
            [](CountDownClock clock) { return clock.count_down > 0; });

        if (Current_control_flow != Process_thread) {
            // fatal("Invalid source of system entry");
            outb(PIC1_COMMAND, EOI_CMD); // send EOI to PIC interrupt controller
            return;
        }

        // save current process status, return to scheduler
        save_context_from_trapframe(CURRENT_PROCESS->context, tf);
        running_queue.remove(CURRENT_PROCESS);
        ready_queue.push_back(CURRENT_PROCESS);
        CURRENT_PROCESS->status = Ready;
        restore_context_to_trapframe(Kernel_proc.context, tf);

        Current_control_flow = Kernel_thread;

        outb(PIC1_COMMAND, EOI_CMD); // send EOI to PIC interrupt controller
    } break;
    case IRQ_KEYBOARD: {
        ps2_keyboard_interrupt();

        // resume process if possible
        if (!gets_enabled) {
            if (gets_count) {
                Process *p = process_waiting_gets;
                p->context.eax = (uint32_t)gets_buffer;

                // TODO: restore to process
                blocking_queue.remove(p);
                ready_queue.push_back(p);
                p->status = Ready;

                gets_count = 0;
                gets_already_count = 0;
                gets_buffer = nullptr;
                process_waiting_gets = nullptr;
            }
        }
    } break;
    default:
        fatal("Unsupported IRQ %d", IRQ);
        break;
    }
}

extern "C" {

void interrupt_handler(TrapFrame *tf) {
    trace("interrupt handler");
    auto condition_code = tf->condition_code;
    auto error_code = tf->error_code;
    auto return_addr = tf->return_addr;

    uint32_t num = condition_code;

    uint8_t *addr = nullptr;
    trace("Interrupt %d, error_code 0x%x, return 0x%x", condition_code,
          error_code, return_addr);
    trace("eax: 0x%x    ebx: 0x%x    ecx: 0x%x    edx:0x%x", tf->eax, tf->ebx,
          tf->ecx, tf->edx);
    trace("esp: 0x%x    ebp: 0x%x    esi: 0x%x    edi:0x%x", tf->unused_esp,
          tf->ebp, tf->esi, tf->edi);
    trace("ds: 0x%x     es: 0x%x     fs: 0x%x     gs:0x%x", tf->ds, tf->es,
          tf->fs, tf->gs);
    trace("Compiled at: " Compilation_datetime);

    switch (condition_code) {
    case 7:
        debug("pass 7");
        break;
    case 8:
        debug("pass 8");
        break;
    case 12: // stack exception
        if (tf->error_code) {
            // due to not-pressent, or overflow
            debug("due to not-pressent, or overflow");
            debug("index: 0x%x, ti: %d, rpl: 0x%x", error_code >> 3,
                  (error_code >> 2) & 0b1, error_code & 0b11);
        } else {
            debug("other reasons");
        }
        break;
    case 14:
        // page fault
        uint32_t data;
        __asm__ __volatile__("movl %%cr2, %0\n\t"
                             : "=r"(data)
                             :);
        debug("page fault due to addr 0x%x", data);
        // p bit
        if (error_code & 1) {
            debug("page level protection violation, ");
        } else {
            debug("not present page, ");
        }
        // r/w bit
        if (error_code & (1 << 1)) {
            debug("write, ");
        } else {
            debug("read, ");
        }
        // u/s bit
        if (error_code & (1 << 2)) {
            debug("user mode.\n");
        } else {
            debug("supervisor mode.\n");
        }

        addr = (uint8_t *)(data & ~0x111);
        add_kernel_memory_mapping(addr, addr);
        break;

    case SYSCALL_INT_ID:
        system_entry(tf->eax, tf);
        break;

    default:
        if (is_hardware_interrupt(condition_code)) {
            hardware_interrupt_handler(condition_code, tf);
            if (return_to_process) {
                Current_control_flow = Process_thread;
                return_to_process = false;
            }
        }
        break;
    }
    trace("return from interrupt_handler");
    return;
}
}

const int IDT_size = 256;
Gate_Descriptor IDT[IDT_size];
extern uint32_t vectors[];
uint16_t IDTR[3];

void init_interrupt_handler() {
    debug("IDE address 0x%x", (int)&IDT[0]);
    debug("vectors address 0x%x", (int)&vectors[0]);
    Gate_Descriptor desc; // interrupt gate
    uint32_t handler_pointer = 0;

    desc.offset_low = handler_pointer;
    desc.offset_high = handler_pointer >> 16;
    desc.selector = 0x0008; // points to GDT[1] in bootloader.S
    desc.reserved = 0b00000000;
    desc.type = 0b01110;
    desc.dpl = 0b00;
    desc.p = 0b1;

    for (int i = 0; i < IDT_size; i++) {
        handler_pointer = vectors[i];
        desc.offset_low = handler_pointer;
        desc.offset_high = handler_pointer >> 16;

        if (i == 32) { // for software interrupt from ring 3
            desc.dpl = 0b11;
        }

        IDT[i] = desc;
    }

    // set IDTR
    IDTR[0] = sizeof(Gate_Descriptor) * IDT_size - 1; // limit
    // TODO: is it valid to ignore 0xFFFF below?
    IDTR[1] = ((uint32_t)IDT) & 0xFFFF; // offset
    IDTR[2] = ((uint32_t)IDT) >> 16;
    __asm__ __volatile__("lidt %0\n\t"
                         :
                         : "m"(IDTR));
    __asm__ __volatile__("sti\n\t"
                         :
                         :);
}

#include "bootloader32.h"
#include "kernel.h"

extern "C" {

struct TrapFrame { // order should be opposite to alltraps.S
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp; // status inside alltraps, useless
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t condition_code;
    uint32_t error_code;
    uint32_t return_addr;
};

void interrupt_handler(TrapFrame *tf) {
    auto condition_code = tf->condition_code;
    auto error_code = tf->error_code;
    auto return_addr = tf->return_addr;

    uint32_t num = condition_code;

    if (condition_code == 8) {
        debug("interrupt_handler %d\n", 8);
        return;
    }

    uint8_t *addr = nullptr;
    debug("Interrupt %d, error_code 0x%x, return 0x%x", condition_code, error_code, return_addr);
    debug("eax: 0x%x    ebx: 0x%x    ecx: 0x%x    edx:0x%x", tf->eax, tf->ebx, tf->ecx, tf->edx);
    debug("esp: 0x%x    ebp: 0x%x    esi: 0x%x    edi:0x%x", tf->esp, tf->ebp, tf->esi, tf->edi);
    debug("ds: 0x%x     es: 0x%x     fs: 0x%x     gs:0x%x", tf->ds, tf->es, tf->fs, tf->gs);

    switch (condition_code) {
    case 7:
        debug("pass 7");
        return;
        break;
    case 12: // stack exception
        debug("pass 12");
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
        add_paging_map(addr, addr);
        return;
        break;

    default:
        break;
    }

    panic("This is interrupt_handler %d 0x%x\n", condition_code, error_code);
}
}

// Page 157. Figure 9-3
struct Gate_Descriptor {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t type : 5;
    uint8_t dpl : 2;
    uint8_t p : 1; // 1 for present, 0 for not-present exception
    uint16_t offset_high;
} __attribute__((packed));

const int IDT_size = 256;
Gate_Descriptor IDT[IDT_size];
extern uint32_t vectors[];
uint16_t IDTR[3];

void init_interrupt_handler() {
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
        IDT[i] = desc;

        handler_pointer = vectors[i];
        desc.offset_low = handler_pointer;
        desc.offset_high = handler_pointer >> 16;
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

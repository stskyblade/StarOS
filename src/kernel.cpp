#include "kernel.h"
#include "bootloader32.h"

extern "C" {
extern void interrupt_handler_0();
extern void interrupt_handler_1();
extern void interrupt_handler_2();
extern void interrupt_handler_3();
extern void interrupt_handler_4();
extern void interrupt_handler_5();
extern void interrupt_handler_6();
extern void interrupt_handler_7();
extern void interrupt_handler_8();
extern void interrupt_handler_9();
extern void interrupt_handler_10();
extern void interrupt_handler_11();
extern void interrupt_handler_12();
extern void interrupt_handler_13();
extern void interrupt_handler_14();
extern void interrupt_handler_15();
extern void interrupt_handler_16();
extern void interrupt_handler_17();
extern void interrupt_handler_18();
extern void interrupt_handler_19();
extern void interrupt_handler_20();
extern void interrupt_handler_21();
extern void interrupt_handler_22();
extern void interrupt_handler_23();
extern void interrupt_handler_24();
extern void interrupt_handler_25();
extern void interrupt_handler_26();
extern void interrupt_handler_27();
extern void interrupt_handler_28();
extern void interrupt_handler_29();
extern void interrupt_handler_30();
extern void interrupt_handler_31();
extern void interrupt_handler_32();
extern void interrupt_handler_33();
extern void interrupt_handler_34();
extern void interrupt_handler_35();
extern void interrupt_handler_36();
extern void interrupt_handler_37();
extern void interrupt_handler_38();
extern void interrupt_handler_39();
extern void interrupt_handler_40();
extern void interrupt_handler_41();
extern void interrupt_handler_42();
extern void interrupt_handler_43();
extern void interrupt_handler_44();
extern void interrupt_handler_45();
extern void interrupt_handler_46();
extern void interrupt_handler_47();
extern void interrupt_handler_48();
extern void interrupt_handler_49();
extern void interrupt_handler_50();
extern void interrupt_handler_51();
extern void interrupt_handler_52();
extern void interrupt_handler_53();
extern void interrupt_handler_54();
extern void interrupt_handler_55();
extern void interrupt_handler_56();
extern void interrupt_handler_57();
extern void interrupt_handler_58();
extern void interrupt_handler_59();
extern void interrupt_handler_60();
extern void interrupt_handler_61();
extern void interrupt_handler_62();
extern void interrupt_handler_63();
extern void interrupt_handler_64();
extern void interrupt_handler_65();
extern void interrupt_handler_66();
extern void interrupt_handler_67();
extern void interrupt_handler_68();
extern void interrupt_handler_69();
extern void interrupt_handler_70();
extern void interrupt_handler_71();
extern void interrupt_handler_72();
extern void interrupt_handler_73();
extern void interrupt_handler_74();
extern void interrupt_handler_75();
extern void interrupt_handler_76();
extern void interrupt_handler_77();
extern void interrupt_handler_78();
extern void interrupt_handler_79();
extern void interrupt_handler_80();
extern void interrupt_handler_81();
extern void interrupt_handler_82();
extern void interrupt_handler_83();
extern void interrupt_handler_84();
extern void interrupt_handler_85();
extern void interrupt_handler_86();
extern void interrupt_handler_87();
extern void interrupt_handler_88();
extern void interrupt_handler_89();
extern void interrupt_handler_90();
extern void interrupt_handler_91();
extern void interrupt_handler_92();
extern void interrupt_handler_93();
extern void interrupt_handler_94();
extern void interrupt_handler_95();
extern void interrupt_handler_96();
extern void interrupt_handler_97();
extern void interrupt_handler_98();
extern void interrupt_handler_99();
extern void interrupt_handler_100();
extern void interrupt_handler_101();
extern void interrupt_handler_102();
extern void interrupt_handler_103();
extern void interrupt_handler_104();
extern void interrupt_handler_105();
extern void interrupt_handler_106();
extern void interrupt_handler_107();
extern void interrupt_handler_108();
extern void interrupt_handler_109();
extern void interrupt_handler_110();
extern void interrupt_handler_111();
extern void interrupt_handler_112();
extern void interrupt_handler_113();
extern void interrupt_handler_114();
extern void interrupt_handler_115();
extern void interrupt_handler_116();
extern void interrupt_handler_117();
extern void interrupt_handler_118();
extern void interrupt_handler_119();
extern void interrupt_handler_120();
extern void interrupt_handler_121();
extern void interrupt_handler_122();
extern void interrupt_handler_123();
extern void interrupt_handler_124();
extern void interrupt_handler_125();
extern void interrupt_handler_126();
extern void interrupt_handler_127();
extern void interrupt_handler_128();
extern void interrupt_handler_129();
extern void interrupt_handler_130();
extern void interrupt_handler_131();
extern void interrupt_handler_132();
extern void interrupt_handler_133();
extern void interrupt_handler_134();
extern void interrupt_handler_135();
extern void interrupt_handler_136();
extern void interrupt_handler_137();
extern void interrupt_handler_138();
extern void interrupt_handler_139();
extern void interrupt_handler_140();
extern void interrupt_handler_141();
extern void interrupt_handler_142();
extern void interrupt_handler_143();
extern void interrupt_handler_144();
extern void interrupt_handler_145();
extern void interrupt_handler_146();
extern void interrupt_handler_147();
extern void interrupt_handler_148();
extern void interrupt_handler_149();
extern void interrupt_handler_150();
extern void interrupt_handler_151();
extern void interrupt_handler_152();
extern void interrupt_handler_153();
extern void interrupt_handler_154();
extern void interrupt_handler_155();
extern void interrupt_handler_156();
extern void interrupt_handler_157();
extern void interrupt_handler_158();
extern void interrupt_handler_159();
extern void interrupt_handler_160();
extern void interrupt_handler_161();
extern void interrupt_handler_162();
extern void interrupt_handler_163();
extern void interrupt_handler_164();
extern void interrupt_handler_165();
extern void interrupt_handler_166();
extern void interrupt_handler_167();
extern void interrupt_handler_168();
extern void interrupt_handler_169();
extern void interrupt_handler_170();
extern void interrupt_handler_171();
extern void interrupt_handler_172();
extern void interrupt_handler_173();
extern void interrupt_handler_174();
extern void interrupt_handler_175();
extern void interrupt_handler_176();
extern void interrupt_handler_177();
extern void interrupt_handler_178();
extern void interrupt_handler_179();
extern void interrupt_handler_180();
extern void interrupt_handler_181();
extern void interrupt_handler_182();
extern void interrupt_handler_183();
extern void interrupt_handler_184();
extern void interrupt_handler_185();
extern void interrupt_handler_186();
extern void interrupt_handler_187();
extern void interrupt_handler_188();
extern void interrupt_handler_189();
extern void interrupt_handler_190();
extern void interrupt_handler_191();
extern void interrupt_handler_192();
extern void interrupt_handler_193();
extern void interrupt_handler_194();
extern void interrupt_handler_195();
extern void interrupt_handler_196();
extern void interrupt_handler_197();
extern void interrupt_handler_198();
extern void interrupt_handler_199();
extern void interrupt_handler_200();
extern void interrupt_handler_201();
extern void interrupt_handler_202();
extern void interrupt_handler_203();
extern void interrupt_handler_204();
extern void interrupt_handler_205();
extern void interrupt_handler_206();
extern void interrupt_handler_207();
extern void interrupt_handler_208();
extern void interrupt_handler_209();
extern void interrupt_handler_210();
extern void interrupt_handler_211();
extern void interrupt_handler_212();
extern void interrupt_handler_213();
extern void interrupt_handler_214();
extern void interrupt_handler_215();
extern void interrupt_handler_216();
extern void interrupt_handler_217();
extern void interrupt_handler_218();
extern void interrupt_handler_219();
extern void interrupt_handler_220();
extern void interrupt_handler_221();
extern void interrupt_handler_222();
extern void interrupt_handler_223();
extern void interrupt_handler_224();
extern void interrupt_handler_225();
extern void interrupt_handler_226();
extern void interrupt_handler_227();
extern void interrupt_handler_228();
extern void interrupt_handler_229();
extern void interrupt_handler_230();
extern void interrupt_handler_231();
extern void interrupt_handler_232();
extern void interrupt_handler_233();
extern void interrupt_handler_234();
extern void interrupt_handler_235();
extern void interrupt_handler_236();
extern void interrupt_handler_237();
extern void interrupt_handler_238();
extern void interrupt_handler_239();
extern void interrupt_handler_240();
extern void interrupt_handler_241();
extern void interrupt_handler_242();
extern void interrupt_handler_243();
extern void interrupt_handler_244();
extern void interrupt_handler_245();
extern void interrupt_handler_246();
extern void interrupt_handler_247();
extern void interrupt_handler_248();
extern void interrupt_handler_249();
extern void interrupt_handler_250();
extern void interrupt_handler_251();
extern void interrupt_handler_252();
extern void interrupt_handler_253();
extern void interrupt_handler_254();
extern void interrupt_handler_255();
extern void interrupt_handler_end();

void interrupt_handler(uint32_t error_code) {
    uint32_t num = error_code;

    if (error_code == 8) {
        printf("interrupt_handler %d\n", 8);
        return;
    }

    panic("This is interrupt_handler %d\n", error_code);
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
uint16_t IDTR[3];

void init_interrupt_handler() {
    Gate_Descriptor desc; // interrupt gate
    uint32_t handler_pointer = (uint32_t)interrupt_handler_0;
    uint32_t handler_code_size = (((uint32_t)interrupt_handler_end) - handler_pointer) / 256;

    desc.offset_low = handler_pointer;
    desc.offset_high = handler_pointer >> 16;
    desc.selector = 0x0008; // points to GDT[1] in bootloader.S
    desc.reserved = 0b00000000;
    desc.type = 0b01110;
    desc.dpl = 0b00;
    desc.p = 0b1;

    for (int i = 0; i < IDT_size; i++) {
        IDT[i] = desc;

        handler_pointer += handler_code_size;
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

void test_malloc() {
    int *ip = (int *)malloc(sizeof(int));
    if (ip != (int *)0x20200000) {
        panic("failed");
    }

    int *ip2 = (int *)malloc(sizeof(int));
    if (ip2 != (int *)0x20200008) {
        panic("failed");
    }

    char *cp = (char *)malloc(25);
    if (cp != (char *)(0x20200008 + 8)) {
        panic("failed");
    }

    char *cp2 = (char *)malloc(1);
    if (cp2 != (char *)(0x20200008 + 8 + 25 + 7)) {
        panic("failed");
    }

    printf(".");
}

void run_test() {
    printf("Running kernel test:\n");
    test_malloc();
    printf("All test passed.\n");
}

struct PTE {
    uint8_t p : 1;
    uint8_t rw : 1;
    uint8_t user_or_supervisor : 1;
    uint8_t reserved1 : 2; // 00
    uint8_t access : 1;
    uint8_t dirty : 1;
    uint8_t reserved2 : 2; // 00
    uint8_t avail : 3;
    uint32_t address : 20;

} __attribute__((packed));

PTE *kernel_paging_directory = nullptr;

// add a Page Table Entry
// map `linear_address` to `physical_address`, 4KB
// FIXME:linear_address provide DIR index & PTE index. So page table is not added to directory at index 0, 1, 2...
// but index comes from linear_address. 1024 entries match 10-bit dir, 10-bit page
void add_paging_map(uint8_t *linear_address, uint8_t *physical_address) {
    // check directory is valid
    // check directory[dir] is valid
    // check page_table[page] is valid

    if (reinterpret_cast<uint32_t>(linear_address) & 0b111111111111 ||
        reinterpret_cast<uint32_t>(physical_address) & 0b111111111111) {
        // linear_address is not aligned to 4KB boundary
        panic("address is not aligned");
    }

    if (!kernel_paging_directory) {
        // create a new page directory
        kernel_paging_directory = reinterpret_cast<PTE *>(alloc_page());
    }

    PTE dir_entry = kernel_paging_directory[reinterpret_cast<uint32_t>(linear_address) >> 22];
    if (!dir_entry.p) {
        // invalid dir_entry
        PTE *page_table = reinterpret_cast<PTE *>(alloc_page());
        dir_entry.p = 1;
        dir_entry.rw = 1;
        dir_entry.user_or_supervisor = 1;
        dir_entry.reserved1 = 0;
        dir_entry.access = 0;
        dir_entry.dirty = 0;
        dir_entry.reserved2 = 0;
        dir_entry.avail = 0;
        dir_entry.address = reinterpret_cast<uint32_t>(page_table) >> 12; // save high 20bit
    }

    // use dir_entry.address, page part in linear_address to calculate the location of page table entry
    PTE &entry = reinterpret_cast<PTE *>(reinterpret_cast<uint32_t>(dir_entry.address) << 12)[reinterpret_cast<uint32_t>(linear_address) >> 12 & 0b1111111111]; // middle 10bit
    entry.p = 1;
    entry.rw = 1;
    entry.user_or_supervisor = 1;
    entry.reserved1 = 0;
    entry.access = 0;
    entry.dirty = 0;
    entry.reserved2 = 0;
    entry.avail = 0;
    entry.address = reinterpret_cast<uint32_t>(physical_address) >> 12; // save high 20bit
}

// linear memory address map to same physical memory address
void prepare_kernel_paging() {
    struct stat filestatus;
    const char *kernel_filename = "/staros_kernel.elf";
    stat(kernel_filename, &filestatus);
    uint32_t kernel_size = filestatus.st_size;

    uint8_t *buffer = (uint8_t *)malloc(kernel_size);
    memset(buffer, 0, kernel_size);
    fs_read(kernel_filename, buffer);
    ELF_HEADER header = *(ELF_HEADER *)buffer;

    if (header.magic_num == 0x464c457f && header.bitness == 1 && header.endianness == 1 && header.version == 1 && header.ABI == 0 && header.ABI_version == 0 && header.type == 0x02 && header.machine == 0x03 && header.e_version == 1 && header.e_phnum == 3) {
        // valid kernel
        Program_header *program_header_table = (Program_header *)&buffer[header.e_phoff];
        for (uint32_t i = 0; i < header.e_phnum; i++) {
            Program_header phentry = program_header_table[i];
            if (phentry.p_filesz != phentry.p_memsz) {
                panic("Unsupported Program header entry %d.", i);
            } else {
                uint32_t segment_size = phentry.p_memsz;
                uint32_t page_frame_size = 1024 * 4; // 4KB
                uint8_t *base = (uint8_t *)phentry.p_vaddr;
                while (segment_size > page_frame_size) {
                    add_paging_map(base, base);
                    segment_size = segment_size - page_frame_size;
                    base = base + page_frame_size;
                }
                add_paging_map(base, base);
            }
        }
    } else {
        panic("Unsupported kernel file");
    }
}

void enable_kernel_paging() {
    // load kernel paging map to cr3(PDBR)
    __asm__ __volatile__("movl %0, %%cr3\n\t"
                         :
                         : "r"(kernel_paging_directory));

    // enable paging
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t"
                         : "=r"(data)
                         :);
    data = data | 0b1; // set PE bit
    __asm__ __volatile__("movl %0, %%cr0\n\t"
                         :
                         : "r"(data));
    __asm__ __volatile__("jmp $0x0008, $flash_after_enable_paging\n\t"
                         :
                         :);

    __asm__ __volatile__("flash_after_enable_paging:\n\t");
    return;
}

void test_kernel_paging() {
    // check is paging enabled
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t"
                         : "=r"(data)
                         :);
    printf("cr0 PE bit should be 1: %d\n", data & 0b1);

    // access unmapped memory should lead to exception
    uint8_t *p = reinterpret_cast<uint8_t *>(0x40005100);
    *p = 1;
    uint8_t tmp = *p;
    *p = *p + 2;
    // panic("should interrupt, shouldn't see this.");

    // assess mapped memory can be seen, map 40005100 to 50005100, write to 40005100, 50005100 is changed
}

extern "C" {
void kernel_main() {
    // init stack
    uint8_t *STACK_BOTTOM = (uint8_t *)0x20000000 - 1024 * 1024; // leave 1MB free space
    __asm__ __volatile__("mov %0, %%esp\n\t"
                         :
                         : "m"(STACK_BOTTOM));

    // init_stack();
    printf("hello,kernel\n");
    init_interrupt_handler();

    run_test();
    prepare_kernel_paging();
    enable_kernel_paging();
    test_kernel_paging();

    // never return
    while (1) {
        sleep(1);
        printf(".");
        ;
    }
}
}

ENTRY(kernel_main)

/*
Useable RAM in both Qemu and Laptop:
0x100000 to 0x20000000, about 500MB
0x20200000 to 0x40004000, about 500MB
0x40005000 to 0xbffe0000, about 2GB

virutal address sapce:
kernel use 0x40005000+, map to 0x40005000+
process use 0x40005000-, map to kernel allocated pages
*/


/*
Keep same with kernel.h
0x40005000 - 0x4c805000: 200MB, text, text*。权限 AX, 读取和执行
0x4c805000 - 0x52c05000: 100MB, rodata。权限 A，仅读取
0x52c05000 - 0x59005000: 100MB: data, bss。权限 AW，读取和写入.
 */


SECTIONS
{
    .text 0x40005000:
    {
        *(.text)
        *(.text*)
    }

    .rodata 0x4c805000:
    {
        *(.rodata)
    }

    .data 0x52c05000:
    {
        *(.data)
        *(.bss)
    }
}

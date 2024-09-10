ENTRY(kernel_main)

/* 1MB - 201MB: text, text*。权限 AX, 读取和执行
201MB - 301MB: rodata。权限 A，仅读取
301MB - (0x20000000) about 501MB: data, bss。权限 AW，读取和写入

1MB -> 0x100000
201MB -> 0xC900000
301MB -> 0x12D00000 */


SECTIONS
{
    .text 0x100000:
    {
        *(.text)
        *(.text*)
    }

    .rodata 0xC900000:
    {
        *(.rodata)
    }

    .data 0x12D00000:
    {
        *(.data)
        *(.bss)
    }
}

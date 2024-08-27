ENTRY(kernel_main)

/* 1GB- 1GB 200MB: text, text*。权限 AX, 读取和执行
1GB 200MB - 1GB 300MB: rodata。权限 A，仅读取
1GB 300MB - 1GB 500MB: data, bss。权限 AW，读取和写入 */

/* 1GB = 1024 * 1024 * 1024 = 0x40000000 */
/* 1GB 200MB = 1GB + 200MB = 0x40000000 + 1024 * 1024 * 200 = 0x4C800000 */
/* 1GB 300MB = 1GB + 300MB = 0x40000000 + 1024 * 1024 * 300 = 0x52C00000 */
/* 1GB 500MB = 1GB + 500MB = 0x40000000 + 1024 * 1024 * 500 = 0x5F400000 */

SECTIONS
{
    .text 0x40000000:
    {
        *(.text)
        *(.text*)
    }

    .rodata 0x4C800000:
    {
        *(.rodata)
    }

    .data 0x52C00000:
    {
        *(.data)
        *(.bss)
    }
}

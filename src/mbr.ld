ENTRY(mbr_start)


SECTIONS
{
  .text 0x7c00:
  {
    *mbr.o(.text)
    *bootloader.o(.text)
    *bootloader_32.o(.text)
    *(.text)
    *(.data)
    *(.rodata)
}
}

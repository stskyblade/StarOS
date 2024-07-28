ENTRY(mbr_start)


SECTIONS
{
  .text 0x7c00:
  {
    *(.text)
}
}

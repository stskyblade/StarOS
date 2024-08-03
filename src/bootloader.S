// This program is written to 2nd sector in disk
// loaded by mbr.S
// 16bit real mode


// TODO:  find where to load this program into. only 30KB left after 0x0000:0x7c00
//  need to call functions in MBR
//  design section mapping
//  or simplily set CS to 0x7c0 ?


  // 0x7e00



.section .text
.global bootloader_start
.code16
bootloader_start:
  // never return
  lea bootloader_str, %ax
  call print_str
  jmp _halt


bootloader_str:
  .asciz "hello,bootloader"

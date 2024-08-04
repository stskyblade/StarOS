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
  mov $0, %ax
  mov $0, %bx
  mov $0, %cx
  mov $0, %dx

  // set SS:SP
  mov $0x9000, %ax
  mov %ax, %ss
  mov $0xfffc, %ax
  mov %ax, %sp

  mov $0, %bp
  mov $0, %si
  mov $0, %di

  // cs is 0
  mov $0, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  lea bootloader_str, %ax
  call print_str
  call demo_read_disk
  jmp _halt


demo_read_disk:
  // write a disk driver to load disk
  pusha

  

  popa
  ret


.section .data
bootloader_str:
  .asciz "hello,bootloader"

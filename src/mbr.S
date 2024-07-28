// This program should be written to MBR
// BIOS will load it at read address mode


.section .text
.global mbr_start
.code16
mbr_start:
  // echo hello,world
  movw $0xb800, %ax
  movw %ax, %es

  // color green
  mov $0b00000010, %ah
  mov $0, %al


  mov $0, %cx //offset of source char
  mov $0, %di // offset of target char 
_loop:  
  lea hello_world, %bx
  add %cx, %bx
  movb (%bx), %al
  movw %ax, %es:(%di)

  add $1, %cx // next char position
  add $2, %di
  cmp $12, %cx
  jl _loop


  mov $1, %ax
_halt:
  add $1, %ax
  jmp _halt

hello_world:    .asciz "hello,world\n"

// add MBR boot sign
// https://stackoverflow.com/questions/47859273/calculating-padding-length-with-gas-att-directives-for-a-boot-sector
.org 510
.word 0xaa55

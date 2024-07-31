// This program should be written to MBR
// BIOS will load it at read address mode


.section .text
.global mbr_start
.code16
mbr_start:
  movw $0, %ax
  movw %ax, %ds

init_print_char:
  mov $0x8000, %ax
  mov %ax, %gs
  movw $0, %gs:(0)


  // set SS:SP
  mov $0x9000, %ax
  mov %ax, %ss
  mov $0xffff, %ax
  mov %ax, %sp

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

test_print_char:  
  mov $'X', %al
  call print_char
  mov $'Y', %al
  call print_char
  mov $'Z', %al
  call print_char


test_print_hex:
  mov $0x1234, %ax
  call print_hex
  mov $0xfedc, %ax
  call print_hex
  mov $0x6979, %ax
  call print_hex


test_str_len:
  lea test_str_1, %ax
  call str_len
  call print_hex

  lea test_str_2, %ax
  call str_len
  call print_hex

  lea test_str_3, %ax
  call str_len
  call print_hex


  mov $1, %ax
_halt:
  add $1, %ax
  jmp _halt

hello_world:    .asciz "hello,world\n"

print_char:
  // print character to screen, auto move cursor
  // AL: char to print
  push %gs
  push %es
  push %ax
  push %bx
  push %di

// 0x80000 to store variable
  push %ax
  mov $0x8000, %ax
  mov %ax, %gs
  mov %gs:(0), %bx  // position, word, 0-1999
  add $1, %bx
  mov %bx, %gs:(0)
  sub $1, %bx  // add 1 and write back

  mov $2, %ax
  mul %bx // multiple 2 got offset
  mov %ax, %di  // offset of 0xb8000

  mov $0xb800, %ax
  mov %ax, %es
  pop %ax // char
  mov $0b00000100, %ah
  mov %ax, %es:(%di) // write to offset

  // write a blink _ to cursor position
  add $2, %di
  mov $0b10000111, %ah // blink white
  mov $'_', %al
  mov %ax, %es:(%di) // write to cursor

  pop %di
  pop %bx
  pop %ax
  pop %es
  pop %gs

  ret

hex_str:  .asciz "0123456789ABCDEF"

print_hex:
  // print value in hex
  // AX: value to print
  pusha

  mov $0, %dx  // reminder
  mov $16, %bx // divisor

  div %bx
  push %dx
  mov $0, %dx

  div %bx
  push %dx
  mov $0, %dx

  div %bx
  push %dx
  mov $0, %dx

  div %bx
  push %dx
  mov $0, %dx

  lea hex_str, %bx

  pop %di
  mov (%bx, %di), %ax
  call print_char
  pop %di
  mov (%bx, %di), %ax
  call print_char
  pop %di
  mov (%bx, %di), %ax
  call print_char
  pop %di
  mov (%bx, %di), %ax
  call print_char

  popa

  ret

test_str_1: .asciz ""
test_str_2: .asciz "abc"
test_str_3: .asciz "ABCDEFG"

str_len:
  // return length of string in ax
  // AX: addr of string
  // Return:
  // AX: length
  push %bx
  push %di

  mov %ax, %bx // base addr of string
  mov $0, %di // offset
str_len_loop: 
  mov $0, %al
  cmpb (%bx, %di), %al
  jz str_len_end
  inc %di
  jmp str_len_loop
str_len_end:
  mov %di, %ax
  pop %di
  pop %bx
  ret


// add MBR boot sign
// https://stackoverflow.com/questions/47859273/calculating-padding-length-with-gas-att-directives-for-a-boot-sector
.org 510
.word 0xaa55

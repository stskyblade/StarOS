// This program should be written to MBR
// BIOS will load it at read address mode


.section .text
.global mbr_start
.code16
mbr_start:
  // set SS:SP
  mov $0x9000, %ax
  mov %ax, %ss
  mov $0xffff, %ax
  mov %ax, %sp

  mov $0, %ax
  mov %ax, %ds

init_print_char:
  mov $0x8000, %ax
  mov %ax, %gs
  movw $0, %gs:(0)

  call print_regs

  mov $1, %ax
_halt:
  add $1, %ax
  jmp _halt

print_char:
  // print character to screen, auto move cursor
  // AL: char to print
  push %gs
  push %es
  pusha
// 0x80000 to store variable
  push %ax
  mov %al, %cl
  mov $0, %ch // store char in cx
  mov $0x8000, %ax
  mov %ax, %gs
  mov %gs:(0), %bx  // position, word, 0-1999

// if char == \n, move cursor to next start of line
  cmp $'\n', %cl
  jne print_char_end_if
// leave blank in current cursor
  mov $2, %ax
  mul %bx // multiple 2 got offset
  mov %ax, %di  // offset of 0xb8000

  mov $0xb800, %ax
  mov %ax, %es
  mov $0b00000000, %ah
  mov $'', %al
  mov %ax, %es:(%di) // write space to offset

  // bx = bx / 80 * 80 + 80
  mov $0, %dx
  mov %bx, %ax
  mov $80, %bx // divisor
  div %bx
  mul %bx
  add $80, %ax
  mov %ax, %bx
  mov %bx, %gs:(0)
  pop %ax
  popa
  pop %es
  pop %gs
  ret

print_char_end_if:  
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

  popa
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


print_str:
  // print string
  // AX: addr of string
  pusha

  mov %ax, %bx
  mov $0, %di
  call str_len

print_str_loop: 
  cmp %ax, %di
  jnb print_str_end  // if di >= ax, end

  // print char
  push %ax
  mov (%bx, %di), %ax
  mov $0, %ah
  call print_char
  pop %ax

  inc %di
  jmp print_str_loop
print_str_end:  
  popa
  ret


ax_str: .asciz "\nax: 0x"
bx_str: .asciz "    bx: 0x"
cx_str: .asciz "    cx: 0x"
dx_str: .asciz "    dx: 0x"
sp_str: .asciz "\nsp: 0x"
bp_str: .asciz "    bp: 0x"
si_str: .asciz "    si: 0x"
di_str: .asciz "    di: 0x"
// cs_str: .asciz "\ncs: 0x"
//ds_str: .asciz "\n    ds: 0x"
//es_str: .asciz "    es: 0x"
// fs_str: .asciz "    fs: 0x"
// gs_str: .asciz "    gs: 0x"
// ss_str: .asciz "    ss: 0x"

print_regs:
  // print all registers
  pusha
  push %di
  push %si
  push %bp
  push %sp
  push %dx
  push %cx
  push %bx
  push %ax

  lea ax_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea bx_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea cx_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea dx_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea sp_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea bp_str, %ax
  call print_str
  pop %ax
  call print_hex

  lea si_str, %ax
  call print_str
  pop %ax
  call print_hex
  
  lea di_str, %ax
  call print_str
  pop %ax
  call print_hex

  popa
  ret

// add MBR boot sign
// https://stackoverflow.com/questions/47859273/calculating-padding-length-with-gas-att-directives-for-a-boot-sector
.org 510
.word 0xaa55

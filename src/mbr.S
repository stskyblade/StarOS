// This program should be written to MBR
// BIOS will load it at 16-bit real address mode


.section .text
.global mbr_start
.code16
mbr_start:
  // set SS:SP
  mov $0x9000, %ax
  mov %ax, %ss
  mov $0xfffc, %ax
  mov %ax, %sp

  mov $0, %ax
  mov %ax, %ds

init_print_char:
  mov $0x8000, %ax
  mov %ax, %gs
  movw $0, %gs:(0)


  // load disk sector 2 to memory
  call load_bootloader
jmp_bootloader:
  jmp bootloader_start  // bootloader_start

  mov $1, %ax
.global _halt
_halt:
  add $1, %ax
  jmp _halt

.global print_char
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
  push %ds

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
  mov $0, %ax
  mov %ax, %ds
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

  pop %ds
  popa

  ret

str_len:
  // return length of string in ax
  // DS:AX: addr of string
  // Return:
  // AX: length
  push %bx
  push %di

  mov %ax, %bx // base addr of string
  mov $0, %di // offset
str_len_loop:
  mov $0, %al
  cmpb (%bx, %di), %al  // 依赖 ds
  jz str_len_end
  inc %di
  jmp str_len_loop
str_len_end:
  mov %di, %ax

  pop %di
  pop %bx
  ret


.global print_str
print_str:
  // print string
  // DS:AX: addr of string
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

print_memory:
  // print memory content at address es:ax, length: bx
  // es: base address
  // ax: offset
  // bx: length
  pusha
  mov $0, %di
  mov %ax, %bp
print_memory_loop:
  cmp %bx, %di
  // di < bx, do & loop
  // di >= bx, end loop
  jnb print_memory_loop_end
  movw %es:(%bp, %di), %ax
  // swap ah, al
  mov %ah, %cl
  mov %al, %ch
  mov %cx, %ax
  call print_hex  // print value
  mov $' ', %al
  mov $0, %ah
  call print_char  // print ' '
  // if di % 16 == 14, print \n
  mov $0, %dx
  mov %di, %ax
  mov $16, %cx
  div %cx
  // remainder in dx
  cmp $14, %dx
  jne not_print_newline
  mov $0, %ah
  mov $'\n', %al
  call print_char

not_print_newline:
  add $2, %di
  jmp print_memory_loop
print_memory_loop_end:
  popa
  ret

load_bootloader:
  // load bootloader from sector 2 to 0x7e00
  // https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
  // read sector 0 to 0x7c00 + 512
// Set AH = 2
// AL = total sector count (0 is illegal) -- cannot cross ES page boundary, or a cylinder boundary, and must be < 128
// CH = cylinder & 0xff
// CL = Sector | ((cylinder >> 2) & 0xC0);   count from 1
// DH = Head -- may include two more cylinder bits
// ES:BX -> buffer
// Set DL = "drive number" -- typically 0x80, for the "C" drive
// Issue an INT 0x13.
  pusha
  push %es
  mov $2, %ah
  mov $63, %al // sector count, 0-64 first cylinder
  mov $0, %ch
  mov $2, %cl  // second sector
  mov $0, %dh
  mov $0x7e0, %bx
  mov %bx, %es
  mov $0, %bx
  mov $0x80, %dl
  int $0x13

  pop %es
  popa
  ret

// add MBR boot sign
// https://stackoverflow.com/questions/47859273/calculating-padding-length-with-gas-att-directives-for-a-boot-sector
.org 510
.word 0xaa55

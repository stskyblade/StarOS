// This program is written to 2-2048 sector in disk
// loaded by mbr.S
// 16bit real mode
// enter 32bit protected mode


// TODO:  find where to load this program into. only 30KB left after 0x0000:0x7c00
//  need to call functions in MBR
//  design section mapping
//  or simplily set CS to 0x7c0 ?


  // 0x7e00



.section .text
.global bootloader_start
.code16

// clear all registers, prepare to enter protected mode
bootloader_start:
  mov $0, %ax
  mov $0, %bx
  mov $0, %cx
  mov $0, %dx

  // set SS:SP
  mov $0x7000, %ax
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
  call detect_memory

jmp_32:
  cli
  // jmp to bootloader_32, never return
  // set up GDT
  lea GDT, %di
  add $8, %di // GDT[1] kernel code segment, https://wiki.osdev.org/GDT_Tutorial
  // write segment descriptor to GDT[1], 8 bytes
// 0000 0000 1100 1111 1001 1010 0000 0000
// 0000 0000 0000 0000 1111 1111 1111 1111
  movw $0xffff, (%di)
  add $2, %di
  movw $0x0000, (%di)
  add $2, %di
  movw $0x9A00, (%di)
  add $2, %di
  movw $0x00cf, (%di)
  // GDT[2]  kernel data segment
  add $2, %di
  movw $0xffff, (%di)
  add $2, %di
  movw $0x0000, (%di)
  add $2, %di
  movw $0x9200, (%di)
  add $2, %di
  movw $0x00cf, (%di)

  mov $0, %di
  // set up GDTR
  lea GDT_end, %ax
  lea GDT, %bx
  sub %bx, %ax
  lea GDTR, %di
  movw %ax, (%di)  // write 2 bytes limit
  add $2, %di
  lea GDT, %ax
  movw %ax, (%di) // write 2 bytes base
  sub $2, %di
  lgdt (%di)

  // enter protected mode
  mov $0, %eax
  mov %cr0, %eax
  or $1, %eax
  mov %eax, %cr0

  jmp $0x0008, $flush


// detect memory layout, find out which region is free to use
detect_memory:
  pusha
  push %es
// point ES:DI at the destination buffer for the list
  lea memory_map_table, %di
//  Clear EBX
  mov $0, %ebx
// Set EDX to the magic number 0x534D4150
  mov $0x534D4150, %edx
_detect_memory_loop:
// Set EAX to 0xE820 (note that the upper 16-bits of EAX should be set to 0)
  mov $0x0000E820, %eax
//  Set ECX to 24
  mov $24, %ecx
// Do an INT 0x15
  int $0x15

  // result
// EAX will be set to 0x534D4150
  cmp $0x534D4150, %eax
  jnz detect_memory_failed


// // Carry flag will be clear
// // EBX will be set to some non-zero value
//   lea bx_str, %ax
//   call print_str
//   mov %bx, %ax
//   call print_hex
//   mov $0, %ah
//   mov $' ', %al
//   call print_char
// // CL will contain the number of bytes actually stored at ES:DI (probably 20).
//   mov $0, %ah
//   mov %cl, %al
//   call print_hex
//   mov $':', %al
//   call print_char

  // // print entry value
  // push %bx
  // mov $0, %ax
  // mov %ax, %es
  // // first 8 bytes
  // mov %di, %ax
  // mov $8, %bx
  // call print_memory
  // mov $'|', %al
  // call print_char
  // // next 8 bytes
  // mov %di, %ax
  // add $8, %ax
  // call print_memory
  // mov $'|', %al
  // call print_char
  // // next 4 bytes
  // mov %di, %ax
  // add $16, %ax
  // mov $4, %bx
  // call print_memory
  // mov $'\n', %al
  // call print_char
  // pop %bx

  // print each di
  // mov %di, %ax
  // call print_hex
  // mov $' ', %ax
  // call print_char

  cmp $0, %ebx
  jz detect_memory_ok
  // if ebx != 0, loop
  add $24, %di
  jmp _detect_memory_loop

  jmp detect_memory_ok
detect_memory_failed:
  lea failed_str, %ax
  call print_str
detect_memory_ok:
  pop %es
  popa
  ret


.code32
flush:
  mov $3, %eax
  mov $4, %ebx
  add %ebx, %eax
  // set SS, DS, ES, FS, GS
  mov $0x0010, %eax
  mov %eax, %ss
  mov $0x7fffc, %esp
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %fs
  mov %eax, %gs

  // lea bootloader_str, %ax
  // call print_str
  // call demo_read_disk
  jmp $0x0008, $bootloader32_start


demo_read_disk:
  pusha
  // primary bus, master drive
  mov $0xa0, %al
  mov $0x1f0, %bx
  call read_disk

  // primary bus, slave drive
  mov $0xb0, %al
  mov $0x1f0, %bx
  call read_disk

  // secondary bus, master drive
  mov $0xa0, %al
  mov $0x170, %bx
  call read_disk

  // secondary bus, slave drive
  mov $0xb0, %al
  mov $0x170, %bx
  call read_disk
  popa
  ret


read_disk:
  // AL: drive select number, 0xa0 for master, 0xb0 for slave
  // BX: base address of registers, 0x1f0 for primary, 0x170 for secondary
  pusha
  push %es

  // print value of AL & BX
  mov $0, %ah
  call print_hex
  push %ax
  mov $' ', %ax
  call print_char
  mov %bx, %ax
  call print_hex
  mov $' ', %ax
  call print_char
  pop %ax

  // select drive
  mov %bx, %dx
  add $6, %dx // drive select register
  out %al, %dx
  // wait 400ns, read Status register 15 times
  mov %bx, %dx
  add $7, %dx // Status register
  mov $15, %cx
read_status_15_loop:
  in %dx, %al
  loop read_status_15_loop

  // Floating bus test
  // if status byte == 0xff, failed
//   cmp $0xff, %al
//   jne floating_test_ok
//   lea floating_test_failed_str, %ax
//   call print_str
//   jmp read_disk_end

// floating_test_ok:
//   lea floating_test_ok_str, %ax
//   call print_str
//   jmp read_disk_end

  // random value r/w test
//   mov %bx, %dx
//   add $2, %dx
//   mov $0xa3, %al
//   out %al, %dx  // write 0xa3 to 0x1F2
//   mov $0, %al
//   in %dx, %al
//   cmp $0xa3, %al
//   je _random_value_1_ok // if al != 0xa3, failed
//   mov $0, %ah
//   call print_hex
//   lea random_value_1_failed_str, %ax
//   call print_str
//   jmp read_disk_end

// _random_value_1_ok:
//   mov %bx, %dx
//   add $5, %dx
//   mov $0xb1, %al
//   out %al, %dx  // write 0xb1 to 0x1F5
//   mov $0, %al
//   in %dx, %al
//   cmp $0xb1, %al
//   je _random_value_2_ok
//   lea random_value_2_failed_str, %ax
//   call print_str
//   jmp read_disk_end

// _random_value_2_ok:
//   lea read_disk_ok_str, %ax
//   call print_str

  // IDENTIFY test
_identify_test:
  mov $0, %al
  mov %bx, %dx
  add $2, %dx
  out %al, %dx  // send 0 to 0x1f2, 3, 4, 5

  add $1, %dx
  out %al, %dx
  add $1, %dx
  out %al, %dx
  add $1, %dx
  out %al, %dx

  add $2, %dx
  mov $0xec, %al
  out %al, %dx  // send IDENTIFY to Command IO port
  in %dx, %al  // if al == 0, drive doesn't exist
  cmp $0, %al
  jne _drive_exist
  lea drive_failed_str, %ax
  call print_str
  jmp read_disk_end

_drive_exist:
  // poll until bit 7 BSY clears

  // if 0x1f4, 0x1f5 are non-zero, is not ATA device
  mov %bx, %dx
  add $4, %dx
  in %dx, %al
  cmp $0, %al
  // if 0x1f4 != 0, not ATA device
  jz _check_next_ata
  lea not_ata_str, %ax
  call print_str
  jmp read_disk_end
_check_next_ata:  // test is 0x1f5 is zero
  inc %dx
  in %dx, %al
  cmp $0, %al
  // if al != 0, not ATA device
  jz _is_ata
  lea not_ata_str, %ax
  call print_str
  jmp read_disk_end

_is_ata:
  mov %bx, %dx
  add $7, %dx
  in %dx, %al
  test $0x80, %al
  jnz _drive_exist  // poll until BSY clears

_poll_until_DRQ_ERR:
  in %dx, %al
  test $9, %al
  // if DRQ or ERR sets, stop polling
  jz _poll_until_DRQ_ERR

  // ERR is clear, read data
  test $1, %al
  jz _err_clear
  lea err_not_clear_str, %ax
  call print_str
  jmp read_disk_end

_err_clear:  // DRQ is set, read IDENTIFY result
  lea disk_sector_buffer, %ax
  mov %ax, %es
  mov $0, %di
  mov %bx, %dx
  mov $256, %cx
  rep insw
  mov $'\n', %ax
  call print_char
  push %bx
  // mov $0, %ax
  // mov $160, %bx
  // call print_memory

  mov $'\n', %ax
  call print_char
  mov $120, %ax
  mov $4, %bx
  call print_memory

  mov $'\n', %ax
  call print_char
  mov $200, %ax
  mov $8, %bx
  call print_memory

  // read disk sector 1 to buffer
_read_disk_sector:
  pop %bx
  // select master device. already selected before
  // mov %bx, %dx  // 0x1f0
  // mov $0xe0, %al
  // out %al, %dx

  // read 1 sector at LBA 65
  mov $1, %al
  mov %bx, %dx
  add $2, %dx
  out %al, %dx  // send sector count to 0x1f2
  mov $65, %al
  add $1, %dx
  out %al, %dx  // send low 8bit LBA to 0x1f3, LBA 65
  mov $0, %al
  add $1, %dx
  out %al, %dx  // send mid 8bit LBA to 0x1f4
  add $1, %dx
  out %al, %dx  // send mid 8bit LBA to 0x1f5
  mov $0x20, %al
  add $2, %dx
  out %al, %dx  // send READ command 0x20 to 0x1f7

  // poll
  // ignore first four read of Status reagister
  // https://wiki.osdev.org/ATA_PIO_Mode#x86_Code_Examples
  mov $4, %cx
_ignore_four_BSY:
  in %dx, %al
  test $0x80, %al  // BSY set
  jne _dec_count
  test $8, %al // DRQ set
  jnz _data_ready
_dec_count:
  dec %cx
  cmp $0, %cx
  jne _ignore_four_BSY

_drive_busy:
  in %dx, %al
  test $0x80, %al  // BSY set
  jnz _drive_busy
  test $8, %al  // DRQ set
  jnz _data_ready
  test $0x21, %al // ERR or DF set
  jz _invalid_result  // both DRQ, ERR, DF is clear
  lea drive_err_str, %ax  // ERR or DF set
  call print_str
  jmp read_disk_end

_invalid_result:
  lea invalid_result_str, %ax
  call print_str
  jmp read_disk_end

_data_ready:
  lea disk_sector_buffer, %ax
  mov %ax, %es
  mov $0, %di
  mov %bx, %dx
  mov $256, %cx
  rep insw

  // TODO: delay 400ns to allow drive to set new values of BSY and DRQ
  // TODO: read multi sector

  mov $'\n', %ax
  call print_char
  push %bx
  mov $0, %ax
  mov $160, %bx
  call print_memory
  pop %bx

read_disk_end:
  pop %es
  popa
  ret




.section .data
bootloader_str:
  .asciz "hello,bootloader\n"
floating_test_failed_str:
  .asciz "floating bus test failed.\n"
floating_test_ok_str:
  .asciz "floating bus test OK.\n"
random_value_1_failed_str:
  .asciz "random value 1 failed\n"
random_value_2_failed_str:
  .asciz "random value 2 failed\n"
read_disk_ok_str:
  .asciz "read disk ok\n"
drive_failed_str:
  .asciz "drive not exist\n"
err_not_clear_str:
  .asciz "err not clear\n"
drive_err_str:
  .asciz "drive error\n"
invalid_result_str:
  .asciz "invalid result\n"
not_ata_str:
  .asciz "not ATA device\n"
bx_str:
  .asciz "bx: "
failed_str:
  .asciz "failed.\n"

  // 512 Bytes
disk_sector_buffer:
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
  .word 0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0,0, 0, 0, 0
disk_sector_buffer_end:
  .word 0xaa55
GDT:
// 64 bytes
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
  .word 0, 0, 0, 0, 0, 0, 0, 0
GDT_end:
GDTR:
  // 2 bytes limit
  .word 0
  // 4 bytes base
  .word 0, 0

.global memory_map_table
memory_map_table:
  // 24 bytes for each entry
  // 80 entries
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
  .long 0, 0, 0, 0, 0, 0
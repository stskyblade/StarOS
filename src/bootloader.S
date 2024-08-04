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

  lea bootloader_str, %ax
  call print_str
  call demo_read_disk
  jmp _halt


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
  // poll until bit 7 clears
  in %dx, %al
  and $0b10000000, %al
  cmp $0, %al  // if != 0, repeat
  jne _drive_exist

  // TODO: Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. If so, the drive is not ATA, and you should stop polling

_poll_until_DRQ_ERR:
  in %dx, %al
  and $0b00001001, %al
  cmp $0, %al  // if DRQ or ERR sets, stop polling
  je _poll_until_DRQ_ERR

  // ERR is clear, read data
  in %dx, %al
  and $0b00000001, %al
  cmp $0, %al
  je _err_clear
  lea err_not_clear_str, %ax
  call print_str
  jmp read_disk_end

_err_clear:
  lea disk_sector_buffer, %ax
  mov %ax, %es
  mov $0, %di
  mov %bx, %dx
  mov $256, %cx
  rep insw
  mov $'\n', %ax
  call print_char
  mov $0, %ax
  mov $160, %bx
  call print_memory

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
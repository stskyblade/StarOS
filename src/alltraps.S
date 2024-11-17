.section .text

.global alltraps
alltraps:
# Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal

  # Set up data segments.
  # movw $(SEG_KDATA<<3), %ax
  # movw %ax, %ds
  # movw %ax, %es

  # Call interrupt_handler(tf), where tf=%esp
  pushl %esp
  call interrupt_handler
  addl $4, %esp

  # Return falls through to trapret...
.global trapret
trapret:
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret


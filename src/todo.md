## Process context switching

use software switching

- [x] map kernel to a high address space. like 0x80000000
- [x] execute initialization code of CPP program, kernel only
- [ ] setup user stack
- [ ] setup kernel mappings in process page directory
- [ ] process execv, software switch


vscode
- [x] add shortcut to select next or previouse when quick open file, Ctrl+n/p


## Where to put Stack of user space program?

Solution 1:
define a Stack variable in cpp run time, link to user space program

Solution 2:
kernel defines stack position, linker script of user level program should obey this.

## Interrupt 13: memset write to unmapped sapce

user program is 5.3KB which is larger than a memory page(4KB), malloc doesn't alloc
and map it right.
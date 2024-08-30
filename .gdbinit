add-auto-load-safe-path /home/stskyblade/Project/StarOS/.gdbinit
target remote localhost:1234
symbol-file build/staros_kernel.elf
b kernel_main

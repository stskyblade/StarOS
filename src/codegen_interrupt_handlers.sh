echo "// generated from codegen_interrupt_handlers.sh"
echo ".section .text"

# each handler's binary code size must be equal
for i in $(seq 0 255);
do echo ".global interrupt_handler_${i}
interrupt_handler_${i}:
  push %eax
  mov \$${i}, %eax
  push %eax
  call interrupt_handler
  add \$4, %esp
  pop %eax
  iret";
done

echo ".global interrupt_handler_end
interrupt_handler_end:
"
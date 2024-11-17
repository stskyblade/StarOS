#!/usr/bin/python


def main():
    print("// generated from codegen_interrupt_handlers.py")
    print(".section .text")

    # FIXME: push fake error_code for some condition code
    for i in range(256):
        print(f""".global interrupt_handler_{i}
        interrupt_handler_{i}:
        push ${i}
        jmp alltraps
        """)

    print(""".global interrupt_handler_end
    interrupt_handler_end:
    """)

    print("""# vector table
    .data
    .global vectors
    vectors:""")
    for i in range(256):
        print(f""".long interrupt_handler_{i}""")

if __name__ == "__main__":
    main()

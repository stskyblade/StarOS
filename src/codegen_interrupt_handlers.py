#!/usr/bin/python3


def main():
    print("// generated from codegen_interrupt_handlers.py")
    print(".section .text\n\n")

    # FIXME: push fake error_code for some condition code
    for i in range(256):
        padding_error_code = "" if ((i >= 10 and i <= 14) or i == 17) else "pushl $0\n"
        print(f""".global interrupt_handler_{i}
interrupt_handler_{i}:
""" + padding_error_code +
        f"""pushl ${i}
jmp alltraps\n""")

        # table of function entry
    print("""\n\n# vector table\n\n
    .data
    .global vectors
    vectors:""")
    for i in range(256):
        print(f""".long interrupt_handler_{i}""")

if __name__ == "__main__":
    main()

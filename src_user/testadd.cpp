int main() {

    __asm__ __volatile__("movl $1, %%eax\n\t" ::);
    __asm__ __volatile__("movl $2, %%ebx\n\t" ::);
    __asm__ __volatile__("add %%ebx, %%eax\n\t" ::);
    int a = 3;
    int b = 4;
    int c = 0x12345;
    a = a + 1;
    b = b + 2;
    c = c + 3;
    int d = a + b + c;
    while (true) {
        ;
    }
    return 0;
}

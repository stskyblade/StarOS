// Interrupt number used by system call
constexpr char SYSCALL_INT_ID = 32;

// System Call Numbers, pass by EAX
// Only use six registers: EAX, EBX, ECX, EDX, ESI, EDI
// EAX for syscall number
// Other small arguments are put into these registers from left to right, in
// order
// Return value is put in EAX
constexpr int SYSCALL_HELLOWORLD = 0;
constexpr int SYSCALL_ADD = 1;
constexpr int SYSCALL_STRUCT = 2;
constexpr int SYSCALL_PRINTF = 3;

// Struct for test only
struct TestPack {
    bool a1;
    char a2;
    short a3;
    int a4;
    double a5;
    int a6;
    short a7;
    char a8;
    bool a9;
    short a10;
};

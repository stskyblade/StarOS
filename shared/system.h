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
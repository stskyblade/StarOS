#include "system.h"

// Just print a message
void syscall_test_helloworld();
// Return the sum of two integers
int syscall_test_add(int a, int b);

inline void system_call(int id);
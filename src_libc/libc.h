#include <stdint.h>
#include "system.h"

uint32_t strlen(const char *str);

// Just print a message
void syscall_test_helloworld();
// Return the sum of two integers
int syscall_test_add(int a, int b);
// Print each field of struct
void syscall_test_struct_arg(TestPack p);
// additional arguments are not supported yet
int printf(char *format, ...);

inline void system_call(int id);
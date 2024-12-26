# C++ const global variable: where does it exists in? when does it be initialized?

Recently, I encountered a bug while programming. Let me describe it as below:

```cpp
#include <iostream>
using namespace std;

const int int1 = 1;
const int int2 = 2;
const int cint1 = int1 + int2;
const int cint2 = int1 - int2;
const int cint3 = cint1 + cint2;

char *const not_used_memory_start = (char *)0x20200000; // to 0x40004000, 500MB
char *const text_memory_start = (char *)0x40005000;
const uint32_t text_memory_length = 0xc800000; // 200MB
char *const rodata_memory_start =
    (char *)text_memory_start + text_memory_length;
const uint32_t rodata_memory_length = 0x6400000; // 100MB
char *const data_memory_start =
    (char *)rodata_memory_start + rodata_memory_length;

int main() {
    cout << "address of int1: " << &int1 << endl;
    cout << "address of int2: " << &int2 << endl;
    cout << "address of cint1: " << &cint1 << endl;
    cout << "address of cint2: " << &cint2 << endl;
    cout << "address of cint3: " << &cint3 << endl;
    return 0;
}
```

`rodata_memory_start` and `data_memory_start` are two global variables.
They have similar definition: same type, similiar initializer. But there
are allocated in different section in ELF file: rodata_memory_start is in
.rodata section with value of 0x000000004c805000 while data_memory_start is in .bss section with value of 0x0000000000000000.

I used to think these two global const variable should be evaluated at compile time if turn on compiler optimization, and should be placed into .rodata section.
I think these two variable should be evaluated at run time before function main and be placed into .data section if I turn off compiler optimization.
But things go wrong. So, which section does const global variable exist? When are they initialized?

##

> All non-local variables with static storage duration are initialized as part of program startup, before the execution of the main function begins (unless deferred, see below).
> https://en.cppreference.com/w/cpp/language/initialization

Apparently, these two variables are global, are "non-local". What is static
storage duration? Are they?

> The storage duration is the property of an object that defines the minimum potential lifetime of the storage containing the object. The storage duration is determined by the construct used to create the object and is one of the following:
>
> static storage duration
> thread storage duration
> (since C++11)
> automatic storage duration
> dynamic storage duration
> ...
> Static storage duration
> A variable satisfying all following conditions has static storage duration ﻿:
>
> It belongs to a namespace scope or are first declared with static or extern.
> It does not have thread storage duration.
> (since C++11)
> The storage for these entities lasts for the duration of the program.
> https://en.cppreference.com/w/cpp/language/storage_duration

These two variable are global, belongs to `global namespace scope`, and don't
have thread storage duration. So they are all static storage duration.

> initialization occurs in two distinct stages:
> Static initialization
> There are two forms of static initialization:
>
> 1) If possible, constant initialization is applied.
> 2) Otherwise, non-local static and thread-local variables are zero-initialized.
> In practice:
>
> Constant initialization is usually applied at compile time. Pre-calculated object representations are stored as part of the program image. If the compiler doesn't do that, it must still guarantee that the initialization happens before any dynamic initialization.
> Variables to be zero-initialized are placed in the .bss segment of the program image, which occupies no space on disk and is zeroed out by the OS when loading the program.
> Dynamic initialization
> After all static initialization is completed, dynamic initialization of non-local variables occurs in the following situations:
>
> 1) Unordered dynamic initialization, which applies only to (static/thread-local) class template static data members and variable templates(since C++14) that aren't explicitly specialized. Initialization of such static variables is indeterminately sequenced with respect to all other dynamic initialization except if the program starts a thread before a variable is initialized, in which case its initialization is unsequenced(since C++17). Initialization of such thread-local variables is unsequenced with respect to all other dynamic initialization.
> 2) Partially-ordered dynamic initialization, which applies to all inline variables that are not an implicitly or explicitly instantiated specialization. If a partially-ordered V is defined before ordered or partially-ordered W in every translation unit, the initialization of V is sequenced before the initialization of W (or happens-before, if the program starts a thread).
> (since C++17)
> 3) Ordered dynamic initialization, which applies to all other non-local variables: within a single translation unit, initialization of these variables is always sequenced in exact order their definitions appear in the source code. Initialization of static variables in different translation units is indeterminately sequenced. Initialization of thread-local variables in different translation units is unsequenced.
> If the initialization of a non-local variable with static or thread storage duration exits via an exception, std::terminate is called.

Do these two variable be initialized in "static initialization" phase? Rule 1 talks about "constant initalization"?
When does "constant initializatin" happen? And how?

> A variable or temporary object obj is constant-initialized if
>
> either it has an initializer or its default-initialization results in some initialization being performed, and
> its initialization full-expression is a constant expression, except that if obj is an object, that full-expression may also invoke constexpr constructors for obj and its subobjects even if those objects are of non-literal class types(since C++11).
> The effects of constant initialization are the same as the effects of the corresponding initialization, except that it's guaranteed that it is complete before any other initialization of a static or thread-local(since C++11) object begins, and it may be performed at compile time.

Of course, these two variable have an initializer. Are their initialization expression "constant expression"?
Expresssions are `(char *)text_memory_start + text_memory_length` and `(char *)rodata_memory_start + rodata_memory_length`. What is "constant expression"?

I know a similiar term "constexpr". Are they same thing?
No. Though "cn" is not defined with constexpr, it is still an "constant expression".
> int n = 1;
> std::array<int, n> a1;  // error: n is not a constant expression
> const int cn = 2;
> std::array<int, cn> a2; // OK: cn is a constant expression

What is a "constant expresssion"?
> A constant expression is either
> an lvalue(until C++14)a glvalue(since C++14) core constant expression that refers to
> a prvalue core constant expression whose value satisfies the following constraints:

What is lvalue or prvalue? Which one do above expressions belong to?
According an simple method to distinguish, `memory_start + memory_length` can't be placed in the left side of "=" assignment operator, so they are prvalue expression.
Are they "core constant expression"?

> A core constant expression is any expression whose evaluation would not evaluate any one of the following:
> ...
> an lvalue-to-rvalue implicit conversion unless applied to a non-volatile literal-type glvalue that ...
> designates an object that is usable in constant expressions,

"memory_start" is a lvalue expression. There performs an lvalue-to-rvalue implicit conversion when it's used as an operand of operator. The addition expression use "memory_start"'s value, rather than it's address.

> Whenever a glvalue appears as an operand of an operator that requires a prvalue for that operand, the lvalue-to-rvalue, array-to-pointer, or function-to-pointer standard conversions are applied to convert the expression to a prvalue.

There has an lvalue-to-rvalue conversion while violates the rule of "core constant expression". But there is an exception rule: non-volatile, literal-type, glvalue, "usable in constant expression".
"memory_start" is const (non-volatile). It's a literal type, because it's a scalar type, because it's a pointer type.

> Usable in constant expressions
> In the list above, a variable is usable in constant expressions at a point P if
> the variable is
> a constexpr variable, or
> it is a constant-initialized variable
> of reference type or
> of const-qualified integral or enumeration type
"memory_start" is not usable in constant expression because it is a constant-initialized variable but not a "const-qualified integral or enumeration type".

So, the expression "memory_start + memory_length" is not a "constant expression"
So, "data_memory_start" can't be constant initialized
So, it is zero-initialized, and then dynamic initialized

## Static initialization VS dynamic initialization

I think static initialization is done in compile time. And dynamic initialization is done in run time by function named like "GLOBAL_static__xx".
Part of dynamic initialization can be done with static initialization at compile time.
> The compilers are allowed to initialize dynamically-initialized variables as part of static initialization

This how this bug happen. These two variables' initialization expression are all not constant expression, because they used variable of type "pointer", which is not "usable in constant expression".
So they are all dynamic initialized. But compiler seperates them, moves one of them into static initialization. One in .rodata with complete value; One in .bss section, need to be initialized by a function called by runtime environment before main function.
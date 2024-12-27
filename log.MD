
**Date : 27 December 2024 03:19 PM**

So to create a garbage collector in C, I had to go through a lot of articles related to it, and i came to the conclusion that it is not possible to create a fully functional garbage collector in C. so we will implement a conservative garbage collector.

A "conservative garbage collector" is a type of garbage collector in computer science that operates with minimal information about the structure of a program's data, meaning it can reclaim memory by assuming any memory location that looks like a potential pointer is actually a pointer, even if it might not be, leading to potentially leaving some unused memory untouched to avoid mistakenly freeing memory that is still needed by the program

we have a lot of tasks to do. first we need a way to scan the stack.
let's start studying about it.

1. Getting the stack top
    [source](https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html)    
    - Definition: void *__builtin_frame_address(unsigned level)
        - level: Determines the stack frame to query.
            - 0 gives the frame address of the current function.
            - 1 gives the frame address of the caller of the current function.
            - 2 gives the frame address of the caller's caller, and so on.

    - Why pass 1?
        - The current function's frame (level = 0) is part of the active stack, so using it might give a value too far down the stack.
        - The caller’s frame (level = 1) is higher up in the stack and provides a better estimate of the starting (or topmost) stack location.
    
    ```c
    void *stack_top = __builtin_frame_address(1);
    ```
    - This code snippet gets the top of the stack by passing 1 to __builtin_frame_address.
    - The stack grows downwards, so the top of the stack is the lowest address in the stack.

2. Getting the stack bottom
    - I found this code on internet which uses inline assembly to get the stack bottom.
    - I will study about it and try to understand it.
    - I will also try to find a better way to get the stack bottom.
    ```c
    uint8_t *__rsp;
    void __read_rsp() {
        __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp));
    }
    __read_rsp();
    uint8_t *stack_bottom = __rsp;

    /* or a macro */
    #define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))
    ```

3. Scanning the stack
    - Now as we have the stack top and stack bottom, we can scan the stack.

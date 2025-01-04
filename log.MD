
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


**Date : 4 January 2025 12:37 PM**

for past few days I was studying Computer Networks and hence I was not able to work on the project. Now I am back to work on the project.

What I think is the most crucial part of the project is to have through knowledge of garbage collection and the various algorithms used in it. I will start studying about it in more detail, though I have already read about it but I will go through it again.

### What is Garbage Collection?
- In computer science, garbage collection (GC) is a form of automatic memory management.
- The garbage collector attempts to reclaim memory that was allocated by the program, but is no longer referenced; such memory is called garbage.
- Garbage collection was invented by American computer scientist **John McCarthy** around 1959 to simplify manual memory management in Lisp.
- Garbage collection relieves the programmer from doing manual memory management, where the programmer specifies what objects to de-allocate and return to the memory system and when to do so.


### Advantages of Garbage Collection
GC frees the programmer from manually de-allocating memory. This helps avoid some kinds of errors:

- **Dangling pointers**, which occur when a piece of memory is freed while there are still pointers to it, and one of those pointers is dereferenced. By then the memory may have been reassigned to another use, with unpredictable results.
- **Double free bugs**, which occur when the program tries to free a region of memory that has already been freed, and perhaps already been allocated again.
- **Certain kinds of memory leaks**, in which a program fails to free memory occupied by objects that have become unreachable, which can lead to memory exhaustion.

### Disadvantages of Garbage Collection

- GC uses computing resources to decide which memory to free. Therefore, the penalty for the convenience of not annotating object lifetime manually in the source code is overhead, which can impair program performance.

- The moment when the garbage is actually collected can be unpredictable, resulting in stalls (pauses to shift/free memory) scattered throughout a session. Unpredictable stalls can be unacceptable in real-time environments, in transaction processing, or in interactive programs. Incremental, concurrent, and real-time garbage collectors address these problems, with varying trade-offs. 

### Garbage Collection Algorithms and Strategies

- All the garbage collection algorithms are divided into two categories:
    - Tracing
    - Reference Counting

1. **Tracing**
    - Tracing garbage collection is the most common type of garbage collection, so much so that "garbage collection" often refers to tracing garbage collection, rather than other methods such as reference counting. 
    - The overall strategy consists of determining which objects should be deallocated by tracing the references from the root set of objects (usually global variables, stack variables, and registers) to all reachable objects.  

    - The most common tracing algorithms are:
        - Naive Mark and Sweep
        - Tri Color Mark and Sweep
        - Generational Garbage Collection
        - Copying Garbage Collection
        - Incremental Garbage Collection

    - Also there are some implementation strategies like **Moving and Non-Moving Collectors**.
        - When objects are freed from memory, the memory can be scattered, this can lead to fragmentation. Moving collectors move objects around in memory to avoid fragmentation.

2. **Reference Counting**
    - Reference counting is a simple garbage collection algorithm that works by counting the number of references to an object. When the reference count drops to zero, the object is deallocated.
    - There are a lot of disadvantages of reference counting like it can't handle circular references, it has overhead of maintaining the reference count, etc.


### Let's learn about Mark and Sweep Algorithm

- **Mark and Sweep** is a classic garbage collection algorithm that is used to reclaim memory occupied by objects that are no longer in use by the program.
- The algorithm consists of two phases: the mark phase and the sweep phase.
- The mark phase identifies all objects that are reachable from the root set of objects (usually global variables, stack variables, and registers) and marks them as live.
- The sweep phase then scans the heap and deallocates any objects that are not marked as live.

# Some important points

### 1. is it a pointer ?
- To implement the tracing algorithm, we need to obtain all the pointers (childs) inside an object. Then find the pointers inside the child objects and so on, traversing the whole object graph.
- The problem with C is that once we get a pointer, we can't tell wether it is a pointer or just a simple number.
- So we will use an approach called the **duck test**. If it looks like a duck, swims like a duck, and quacks like a duck, then it probably is a duck. :) [source](https://en.wikipedia.org/wiki/Duck_test)
- if it looks like a pointer and points to a valid memory location in the heap, then we will consider it as a pointer.
- Now this will make our garbage collector a **conservative garbage collector**, which I mentioned in the first log.

### 2. what are the roots ?
- The roots are the starting points from which we begin our tracing algorithm.
- These objects are guaranteed to be live, so we start from these objects and find all the objects that are reachable from these objects.
- Now, higher level languages keep a track of the roots for GC purposes, but in C, we don't have that luxury.

- In C, we must manually scan the stack to find the roots.
- additionally, some objects might be stored in registers, so we need to scan the registers as well. we can use some tricks to push the registers to the stack and then scan the stack.
**Date : 27 December 2024 03:19 PM**
**Edit: 12 July 2025 03:27 PM**

### What is a Garbage Collector?

- A garbage collector (GC) is a form of automatic memory management that attempts to reclaim memory that was allocated by the program but is no longer referenced. 
- In simpler terms, any memory that is no longer referenced by the program is called `garbage` and the garbage collector attempts to reclaim that memory.
- Suppose you malloc an array of integers and forget to free it or if you make the pointer point to some other memory location, then the memory allocated for the array is no longer reachable by the program. This memory is called `garbage` and the garbage collector helps us reclaim it.

Do you need it? 
well...
"A good programmer is someone who always frees their malloced memory" - Abhijit A.M (He is my professor who taught me a course on problem solving using procedural programming in C)
I think we don't need it, because we are good programmers :D

But I was fascinated by the idea of a garbage collector and wanted to implement one in C, because C doesn't have one. 

So to create a garbage collector in C, I had to go through a lot of articles related to it, and i came to the conclusion that:
- It is possible to create a garbage collector in C, yay! but
- It is not possible to create a "fully functional", "general purpose" or in easy words a "full proof" garbage collector in C.

great adjectives right? :D

But you must have the question, why? or why not? worry not, I will give you the answer.

First, we need to understand some basic concepts of garbage collection and how it works in general.

Some terminologies that you should know before we start:
- **Garbage**: Memory that is no longer referenced by the program and can be reclaimed.
- **Root Set**: The set of objects that are guaranteed to be live, such as global variables, stack variables, and registers.
- **Live Object**: An object that is reachable from the root set and can be accessed by the program.
**Unreachable Object**: An object that is not reachable from the root set and can be reclaimed by the garbage collector.

Most of the garbage collectors work on the principle of "tracing" the root set and finding all the reachable objects from it. This is called the **Tracing Garbage Collection**.
You can think of it as a recursive algorithm

1. start from the root set
2. mark all the objects that are reachable from the root set
3 recursively mark all the objects that are reachable from the marked objects
4. continue until no more objects can be marked
5. deallocate or "sweep" all the objects that are not marked    

### Why can't we do this in C?

1. No run time information
    The requirement of a GC, is to first mark all objects in heap that are reachable from the root set and then mark all the objects reachable from those objects (in the heap).
    This essentially requires the GC to disinguish between pointers and non-pointer data types in the heap.

    **The problem**
   - in C, the compiler does not keep metadata about which bits in memory are data types or pointers. i.e looking at a memory location, we can't tell if it is a pointer or just a integer, float, etc.
   - At runtime, everything is just a sequence of bits  
   ```java
    int *ptr = malloc(sizeof(int));
    int value = 42;
    ```
    - At runtime, both might have similar bit patterns, but one is a pointer and the other is an integer. 

   - On the other hand in language like `Java`, every object has a header that contains metadata about the object, such as its type, size, and other information. The JVM uses this metadata to traverse the objects safely. The GC knows exactly what each memory location represents.


2. No root set
    - In C, we don't have a root set. The root set is the set of objects that are guaranteed to be live, such as global variables, stack variables, and registers.
    - In C, we don't have a way to get the root set. We can only get the root set by scanning the stack and registers, which is not a trivial task. (It is architecture dependent and requires inline assembly or compiler built-ins)
    - In languages like `Java`, the JVM keeps track of the root set and provides APIs to access it.

### So what can we do?
We can create somethign called a "conservative garbage collector".

A "conservative garbage collector" is a type of garbage collector in computer science that operates with minimal information about the structure of a program's data, meaning it can reclaim memory by assuming any memory location that looks like a potential pointer is actually a pointer, even if it might not be, leading to potentially leaving some unused memory untouched to avoid mistakenly freeing memory that is still needed by the program

we have a lot of tasks to do. first we need a way to get the root set, this inolves scanning the stack and registers. 

let's start studying about it.

1. Getting the stack top
    
    **Some terminolgies**
    - **Stack Frame**: when a function is called, your cpu or compiler sets up a stack frame for that function.
    the stack frame is a chunk of memory that contains the function's local variables, return address etc.

    - **frame address**: it is the base address or the starting address of the stack frame

    - Here is the [source](https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html)

   - Definition: void \*\_\_builtin_frame_address(unsigned level)

     - level: Determines the stack frame to query.
       - 0 gives the frame address of the current function.
       - 1 gives the frame address of the caller of the current function.
       - 2 gives the frame address of the caller's caller, and so on.
    
    - What is the stack frame?
      - stack frame means the region of the stack that holds the local variables, saved registers, return address, etc for a function call.

   - Why pass 1?
     - 0 will give the address of the current function's stack frame, which is not what we want.
     - 1 will give the address of the caller's stack frame, which in our case will be the main function's stack frame, which is usually the entry point of the program.
     So using 1 inside a initialization function like `gc_init()` gives us main's frame address

   ```c
   void *stack_top = __builtin_frame_address(1);
   ```
   - This code snippet gets the top of the stack by passing 1 to \_\_builtin_frame_address.

2. Getting the stack bottom

   - we can get the stack bottom by calling a function, declaring a local variable, and then taking the address of that variable.
   it will look something like this:

   ```c
    void *stack_bottom(){
         int local_var;
         return &local_var;
    }
    ```
    This works because the local variable will be allocated and the end of the stack

3. Scanning the stack
   - Now as we have the stack top and stack bottom, we can scan the stack.

okay! if you are confused about this stack part. let me explain it to you
- The stack is a region of memory that is used to store local variables, function parameters, and return addresses.
- The stack always grows downwards, meaning that the stack top is at a higher memory address than the stack bottom.
It will look something like this:

```
    +-----------------------------+
    | stack top (higher address)  |
    +-----------------------------+
    |                             |
    |                             |
    |                             |
    |                             |
    |                             |
    +-----------------------------+
    | stack bottom (lower address)|
    +-----------------------------+
            grows downwards
    ```

So when you call the __builtin_frame_address(1) you get the stack top 

and this is how you get the bottom:

```
    -------------------------------
    | stack top (higher address)  |
    -------------------------------
    | main()                      |
    |                             |
    |                             |
    |                             |
    |                             |
    -------------------------------
    | gc_init()'s frame           |
    | &a                          | <- stack_bottom
    -------------------------------


**Date : 4 January 2025 12:37 PM**

for past few days I was studying and hence I was not able to work on the project. Now I am back to work on the project.

What I think is the most crucial part of the project is to have through knowledge of garbage collection and the various algorithms used in it. I will start studying about it in more detail, though I have already read about it but I will go through it again.

Now i'll write a bunch of stuff formally :D you need to bare with it.

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

- we need some additional resources to keep track of the objects in memory, which can lead to increased memory usage.

- The implementation i'm thinking of is called *stop the world* garbage collection, which means that the program is paused while the garbage collector is running. This can lead to performance issues in real-time applications. This is not acceptable in applications that require low latency or real-time performance.

### What are the various strategies for Garbage Collection?

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
   - When objects are freed from memory, the memory can be scattered, this can lead to fragmentation. 
   *fragmentation* is a situation where free memory is not contiguous, that is there are small chunks of free memory scattered throughout the heap. Suppose you want to allocate a large object, but there is no contiguous block of memory available, then you will not be able to allocate the object, even though there is enough free memory available.

   - Moving collectors move objects around in memory to avoid fragmentation. (Hey guys, mehmood here from future, turns out that I implemented a moving collector too :D)

2. **Reference Counting**
   - Reference counting is a simple garbage collection algorithm that works by counting the number of references to an object. When the reference count drops to zero, the object is deallocated.
   - There are a lot of disadvantages of reference counting like it can't handle circular references, it has overhead of maintaining the reference count, etc.


After going through all these, I've decided to implement the **Mark and Sweep** algorithm, which is a tracing garbage collection algorithm. It is the simplest!

### Let's learn about Mark and Sweep Algorithm

- **Mark and Sweep** is a classic garbage collection algorithm that is used to reclaim memory occupied by objects that are no longer in use by the program.
- The algorithm consists of two phases: the `mark` phase and the `sweep` phase.
- The mark phase identifies all objects that are reachable from the root set of objects and marks them as live.
- The sweep phase then scans the heap and deallocates any objects that are not marked as live.

# Some important points

### 1. is it a pointer ?

- To implement the tracing algorithm, we need to obtain all the pointers (childs) inside an object. Then find the pointers inside the child objects and so on, traversing the whole object graph.
- The problem with C is that once we get a pointer, we can't tell wether it is a pointer or just a simple number.
- I explained the problem in detail earlier, you may take a look at it once again.

- So we will use an approach called the **duck test**. If it looks like a duck, swims like a duck, and quacks like a duck, then it probably is a duck. :) [source](https://en.wikipedia.org/wiki/Duck_test)
- if it looks like a pointer and points to a valid memory location in the heap, then we will consider it as a pointer.
- Now this will make our garbage collector a **conservative garbage collector**, which I mentioned in the first log.

### 2. what are the roots ?

- The roots are the starting points from which we begin our tracing algorithm.
- These objects are guaranteed to be live, so we start from these objects and find all the objects that are reachable from these objects.
- Now, higher level languages keep a track of the roots for GC purposes, but in C, we don't have that luxury.

- In C, we must manually scan the stack to find the roots.
- additionally, some objects might be stored in registers, so we need to scan the registers as well. we can use some tricks to push the registers to the stack and then scan the stack. (smartly avoiding to reveal that i already know how to do it :D)

**Date : 4 January 2025 02:50 PM**

### Let's take a look at the algorithm with an example

1. **The Traceable Object**

   - This will be the most important part of our implementation.
   - This will contain all the information about the object, which will be used by the garbage collector.
   - But for now for now we will just have a simple structure. This example is take from Dmitry Soshnikov's blog, with a few modifications.
   - reference: [Dmitry Soshnikov's blog](https://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/)

   ```c
   typedef struct Traceable {
       char *name;

       Node  *left;
       Node  *right;
   } Traceable;
   ```

   - The mark and sweep algorithm has two phases: `The Mark Phase`(marking all the reachable objects) and `The Sweep Phase`(deallocating the unreachable objects).
   - The `left` and `right` fields are pointers to other objects.
   - For that we need a flag. This is where the `marked` flag comes in.
   - The `size` field is used to store the size of the object.
   - we can store these metadata in the object itself, but it is not a good practice. So we will store it in a separate structure.

   ```c
   typedef struct Metadata {
       int marked;
       size_t size;
   } Metadata;
   ```

   - We can maintain a mapping of the object to its metadata using a hash table or a linked list.

2. **The example**

   - Here's a graph for example

   ```c
   /* This function will create the following graph
       a
      / \
     b   c
        / \
       d   e
          / \
         f   g
              \
               h
   */

   Node *createGraph(){
       Node *a = malloc(sizeof(Node));
       Node *b = malloc(sizeof(Node));
       Node *c = malloc(sizeof(Node));
       Node *d = malloc(sizeof(Node));
       Node *e = malloc(sizeof(Node));
       Node *f = malloc(sizeof(Node));
       Node *g = malloc(sizeof(Node));
       Node *h = malloc(sizeof(Node));

       a->left = b;
       a->right = c;

       c->left = d;
       c->right = e;

       e->left = f;
       e->right = g;

       g->right = h;

       return a;
   }
   ```

   - Now we will create a graph using the above function in the main function.

   ```c
   int main(){
       gc_init();
       Node *a = createGraph();
       gc_dump("Allocated Graph");
       return 0;
   }
   ```

   - Notice that we have called `gc_init()` function. This function will initialize the garbage collector, we will look into its implementation later.
   - The `gc_dump()` function will print the graph we created.

   - Now let's take a look at the graph we created.<br>
     <img src="./References/Images/01 - Allocated Graph.png" width="400">

   - Initially, we only have the reference to the root node `a`. We don't know anything else about the sub-nodes of `a`.

   - The `gc_dump()` function will print the graph in the following format:

   ```
   Allocated Graph

   {
       [H] 0x7f8c1b402010 : {marked: 0, size: 16},
       [G] 0x7f8c1b402000 : {marked: 0, size: 16},
       [F] 0x7f8c1b401ff0 : {marked: 0, size: 16},
       [E] 0x7f8c1b401fe0 : {marked: 0, size: 16},
       [D] 0x7f8c1b401fd0 : {marked: 0, size: 16},
       [C] 0x7f8c1b401fc0 : {marked: 0, size: 16},
       [B] 0x7f8c1b401fb0 : {marked: 0, size: 16},
       [A] 0x7f8c1b401fa0 : {marked: 0, size: 16},
   }
   ```

   - The addresses here are not the actual addresses, they are just for representation.
   - The `marked` field is set to 0, which means that the object is not marked. This is because we haven't started the mark phase yet.

   ### What is Garbage?

   - From the viewpoint of a tracing garbage collector, garbage is any object that is not reachable from the root set of objects.

   - Let's see that in our example.
   - Suppose we perform the following operation:

   ```c
   a->right = NULL;
   ```

   - This single operation will make the entire right subtree Garbage.
   - **Note** : It is garbage in a garbage collector's perspective. In pure C, it is a **memory leak.**<br>

   A more formal definition of `memory leak` is : A memory leak occurs when a program allocates memory on the heap (using functions like `malloc()`, `calloc()`, or `realloc()`) but fails to free it (using `free()`), and then loses all references to that memory.


     <img src="./References/Images/02 - Garbage Right Subtree.png" width="400">

3. **Custom Memory Allocator**

   - We will implement a custom memory allocator for our garbage collector.
   - But it will be a simple wrapper around the `malloc()` function.

   ```c
   void *gc_malloc(size_t size){
       void *block = malloc(size);

       void *metadata = malloc(sizeof(Metadata));

       metadata.size = size;
       metadata.marked = 0;

       gc_add_block(block, metadata); /* mapping the block to its metadata */
       gc_add_root(block); /* adding the block to the root set */

       return block;
   }
   ```

   - We can also have a implementation where we store the metadata in the block itself. in that case we will allocate `size + sizeof(Metadata)` bytes of memory.
   - to lookup the metadata, we will just subtract `sizeof(Metadata)` from the block address.
   - Reference: [Garbage Collector in C](https://github.com/sameerkavthekar/garbage-collector)

4. **The Mark Phase**

   - In the main function, we will call the `gc_run()` function.

   ```c
   int main(){
       gc_init();
       Node *a = createGraph();
       gc_dump("Allocated Graph");
       a->right = NULL;
       gc_run();

       return 0;
   }
   ```

   - The `gc_run()` function will look like this:

   ```c
   void gc_run(){
       gc_mark();
       gc_dump("Marked Graph");

       gc_sweep();
       gc_dump("Swept Graph");
   }
   ```

   - we will dump the graph after the mark phase and after the sweep phase, to see the changes.

   - The `gc_mark()` function will look like this:

   ```c
   void gc_mark(){
       map m = gc_get_roots();
       for(int i = 0; i < m.size; i++){
           gc_mark_object(m.objects[i]);
       }
   }

   void gc_mark_object(void *object){
       Metadata *metadata = gc_get_metadata(object);
       if(metadata->marked) return;

       metadata->marked = 1;

       map m = gc_get_children(object);
       for(int i = 0; i < m.size; i++){
           gc_mark_object(m.objects[i]);
       }
   }

   ```

   - we start from the roots and mark all the reachable objects. for each object, we get its children and mark them as well. and so on.
   - By the end of the mark phase, all the reachable objects will be marked. the unreachable objects will have the `marked` field set to 0.
   - The `gc_get_roots()` function and `gc_get_children()` function will be implemented later.
   - The graph after the mark phase will look like this:<br>
     <img src="./References/Images/03 - Marked Graph.png" width="400">

   - The `gc_dump()` function will print the graph in the following format:

   ```
   marked Graph

   {
       [H] 0x7f8c1b402010 : {marked: 0, size: 16},
       [G] 0x7f8c1b402000 : {marked: 0, size: 16},
       [F] 0x7f8c1b401ff0 : {marked: 0, size: 16},
       [E] 0x7f8c1b401fe0 : {marked: 0, size: 16},
       [D] 0x7f8c1b401fd0 : {marked: 0, size: 16},
       [C] 0x7f8c1b401fc0 : {marked: 0, size: 16},
       [B] 0x7f8c1b401fb0 : {marked: 1, size: 16},
       [A] 0x7f8c1b401fa0 : {marked: 1, size: 16},
   }
   ```

   - Marking will not reclaim the memory, it will just mark the objects.
   - This is a relatively faster process as we visit a subset of the objects that are reachable.
   - In contrast, the sweep phase will visit all the objects in the heap.

5. **The Sweep Phase**

   - Since we marked the objects in the mark phase, we can now sweep the heap and deallocate the unmarked objects.
   - The `gc_sweep()` function will look like this:

   ```c
   void gc_sweep(){
       map m = gc_get_objects();
       for(int i = 0; i < m.size; i++){
           void *object = m.objects[i];
           Metadata *metadata = gc_get_metadata(object);
           if(!metadata->marked){
               gc_remove_block(object);
               free(object);
           }
       }
   }
   ```

   - The `gc_get_objects()` function will return all the objects in the heap.
   - The `gc_remove_block()` function will remove the block from the mapping.
   - In the sweep phase, we visit all the objects in the heap and deallocate the unmarked objects who have the `marked` field set to 0.
   - The graph after the sweep phase will look like this:<br>
     <img src="./References/Images/04 - Swept Graph.png" width="400">

   - The `gc_dump()` function will print the graph in the following format:

   ```
   Swept Graph

   {
       [B] 0x7f8c1b401fb0 : {marked: 0, size: 16},
       [A] 0x7f8c1b401fa0 : {marked: 0, size: 16},
   }
   ```

   ### And that's it for the basic implementation of the mark and sweep algorithm!

### C specific steps to implement the garbage collector

1. The `gc_get_children()` function

   - This function will return all the children pointers of an object.
   - The idea we will use here is to scan the memory of the object and find all the pointers.
   - We will use the duck test to determine if a memory location is a pointer.
   - Here is the implementation of the function:

   ```c
   map gc_get_children(void *object){
       map m;
       Metadata *metadata = gc_get_metadata(object);
       size_t size = metadata->size;

       for(int i = 0; i < size; i++){
           uint8_t *p = (uint8_t *)object + i;
           if(gc_contains((uintptr_t *)*(uintptr_t *)p)){
               m.objects[m.size++] = *(uintptr_t *)p;
           }
       }

       return m;
   }
   ```

   - The `gc_contains()` function will check if the memory location is in the set of allocated objects.
   - The `gc_get_metadata()` function will return the metadata of the object.
   - Reason for using `uintptr_t` is that it guarantees that it is large enough to hold a pointer. for example, on a 64-bit system, it will be 8 bytes and on a 32-bit system, it will be 4 bytes.

2. The `gc_get_roots()` function

   - This function will return all the roots.
   - The roots are the starting points from which we begin our tracing algorithm.
   - In C, we must manually scan the stack to find the roots
   - We have already discussed about it in the previous logs.
   - But to push the registers to the stack, we can use the following code:

   ```c
   jmp_buf jb;
   setjmp(jb);
   ```

   - This will push the registers to the stack.
   - We can then scan the stack to find the roots.
   - The `gc_get_roots()` function will look like this:

   ```c
   map gc_get_roots(){
       map m;
       uint8_t *stack_top = (uint8_t *)gc.stack_top;
       uint8_t *stack_bottom = __rsp;

       while(stack_bottom < stack_top){
           if(gc_contains((uintptr_t *)*(uintptr_t *)stack_bottom)){
               m.objects[m.size++] = *(uintptr_t *)stack_bottom;
           }
           stack_bottom++;
       }

       return m;
   }
   ```

   - The `gc.stack_top` is the top of the stack, which we got using the `__builtin_frame_address(1)` function, which we will use in the `gc_init()` function.

### That's it for the basic implementation of the garbage collector in C!

### Now I will start implementing the basic mark and sweep garbage collector in C, we can later improve it by adding more features or using different algorithms.

**Date : 5 January 2025 08:19 PM**

### Let's start implementing the garbage collector

- But to Start we need to implement a HashMap in C. It will be used to store the mapping of the object to its metadata.
- I will implement a good and efficient HashMap in C.

### HashMap in C

- A hashmap is a data structure that stores key-value pairs. It allows for fast retrieval of the values associated with a given key.
- The hashmap is implemented using an array of linked lists. Each element in the array is a linked list that stores the key-value pairs.
- The key is hashed to determine the index in the array where the key-value pair should be stored.
- The hashmap provides fast insertion, deletion, and retrieval of key-value pairs.
- There are various hash functions that can be used to hash the key. The hash function should be efficient and provide a good distribution of keys across the array.
- The hashmap should also handle collisions, where two keys hash to the same index in the array.
- There are various ways to avoid collisions. The approach I am choosing is chaining, where the key-value pairs that hash to the same index are stored in a linked list.
- We will definitely improve the implementation later.
- We will store the pointers as `uintptr_t` in the hashmap.

### HashMap Implementation

1. **The Node Structure**

   - The node structure will store the key-value pair.

   ```c
   typedef struct HashMapNode {
       uintptr_t key;
       uintptr_t value;
       struct HashMapNode *next;
   } HashMapNode;
   ```

   - The `key` will be the pointer to the object and the `value` will be the pointer to the metadata.
   - The `next` field will point to the next node in the linked list.

2. **The HashMap Structure**

   - The hashmap structure will store the array of linked lists.

   ```c
   typedef struct HashMap {
       Node **buckets;
       int size;
   } HashMap;
   ```

   - The `buckets` field is an array of linked lists.
   - The `size` field is the size of the array.

3. **The HashMap Functions**

   1. Hash Functions

      - we will use murmur hash function which is a efficient hash function suitable for general hash-based lookup.
      - The hash function will take the key and the size of the array as input and return the index in the array where the key-value pair should be stored.
      - This implementation is take from [here](https://github.com/PayalSankpal/DSA-BloomFilters/blob/main/murmurhash.c)

      ```c
      uint32_t ROTL32(uint32_t x, int y);
      uint32_t getblock32(const uint32_t *p, int i);
      uint32_t generate_seed(void);
      uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed);
      uintptr_t murmur_hash3(uintptr_t key);
      uintptr_t hash(uintptr_t key, int size);

      /* Hash function for pointer values */
      uintptr_t hash(uintptr_t key, int size) {
          return murmur_hash3(key) % size;
      }

      /* Wrapper for MurmurHash3 */
      uintptr_t murmur_hash3(uintptr_t key) {
          return (uintptr_t)murmurhash3_x86_32(&key, sizeof(uintptr_t), generate_seed());
      }

      /* Seed generator */
      uint32_t generate_seed(void) {
          time_t current_time;
          time(&current_time);
          return (uint32_t)current_time;
      }

      /* MurmurHash3 implementation */
      uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed) {
          const uint8_t *data = (const uint8_t *)key;
          const int nblocks = len / 4;
          uint32_t h1 = seed;
          const uint32_t c1 = 0xcc9e2d51;
          const uint32_t c2 = 0x1b873593;

          /* Body */
          const uint32_t *blocks = (const uint32_t *)(data);
          for (int i = 0; i < nblocks; i++) {
              uint32_t k1 = getblock32(blocks, i);

              k1 *= c1;
              k1 = ROTL32(k1, 15);
              k1 *= c2;

              h1 ^= k1;
              h1 = ROTL32(h1, 13);
              h1 = h1 * 5 + 0xe6546b64;
          }

          /* Tail*/
          const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
          uint32_t k1 = 0;

          switch (len & 3) {
              case 3:
                  k1 ^= tail[2] << 16;
                  /* fallthrough */
              case 2:
                  k1 ^= tail[1] << 8;
                  /* fallthrough */
              case 1:
                  k1 ^= tail[0];
                  k1 *= c1;
                  k1 = ROTL32(k1, 15);
                  k1 *= c2;
                  h1 ^= k1;
          }

          /* Finalization */
          h1 ^= len;
          h1 ^= h1 >> 16;
          h1 *= 0x85ebca6b;
          h1 ^= h1 >> 13;
          h1 *= 0xc2b2ae35;
          h1 ^= h1 >> 16;

          return h1;
      }

      /* Helper function to get a block */
      uint32_t getblock32(const uint32_t *p, int i) {
          return p[i];
      }

      /* Bit rotation helper */
      uint32_t ROTL32(uint32_t x, int y) {
          return (x << y) | (x >> (32 - y));
      }
      ```

   2. HashMap Initialization

      - The `hashmap_init()` function will initialize the hashmap.

      ```c
      void hashmap_init(HashMap *map){
          map->buckets = malloc(HASHMAP_SIZE * sizeof(Node *));
          map->size = HASHMAP_SIZE;

          for(int i = 0; i < HASHMAP_SIZE; i++){
              map->buckets[i] = NULL;
          }
      }
      ```

   3. HashMap Insertion

      - The `hashmap_insert()` function will insert a key-value pair into the hashmap.

      ```c
      void hashmap_insert(HashMap *map, uintptr_t key, uintptr_t value){
          uintptr_t index = hash(key, map->size);

          Node *node = malloc(sizeof(Node));
          node->key = key;
          node->value = value;
          node->next = map->buckets[index];
          map->buckets[index] = node;
      }
      ```

   4. HashMap Lookup

      - The `hashmap_lookup()` function will lookup the value associated with a given key.

      ```c
      uintptr_t hashmap_lookup(HashMap *map, uintptr_t key){
          uintptr_t index = hash(key, map->size);

          Node *node = map->buckets[index];
          while(node){
              if(node->key == key){
                  return node->value;
              }
              node = node->next;
          }

          return 0;
      }
      ```

   5. HashMap Deletion

      - The `hashmap_delete()` function will delete a key-value pair from the hashmap.

      ```c
      void hashmap_delete(HashMap *map, uintptr_t key){
          uintptr_t index = hash(key, map->size);

          Node *node = map->buckets[index];
          Node *prev = NULL;

          while(node){
              if(node->key == key){
                  if(prev){
                      prev->next = node->next;
                  } else {
                      map->buckets[index] = node->next;
                  }
                  free(node);
                  return;
              }
              prev = node;
              node = node->next;
          }
      }
      ```

   6. HashMap Free

      - The `hashmap_free()` function will free the hashmap.

      ```c
      void hashmap_free(HashMap *map){
          for(int i = 0; i < map->size; i++){
              Node *node = map->buckets[i];
              while(node){
                  Node *temp = node;
                  node = node->next;
                  free(temp);
              }
          }
          free(map->buckets);
          free(map);
      }
      ```

   7. HashMap Iterator

   ```c
       HashMapIterator *hashmap_iterator_create(HashMap *map){
           HashMapIterator *iter = malloc(sizeof(HashMapIterator));
           iter->map = map;
           iter->index = 0;
           iter->node = NULL;

           while(iter->index < map->size && !map->buckets[iter->index]){
               iter->index++;
           }

           if(iter->index < map->size){
               iter->node = map->buckets[iter->index];
           }

           return iter;
       }

       int hashmap_iterator_has_next(HashMapIterator *iter){
           return iter->node != NULL;
       }

       int hashmap_iterator_next(HashMapIterator *iter, uintptr_t *key, uintptr_t *value){
           if(!hashmap_iterator_has_next(iter)) return 0;

           *key = iter->node->key;
           *value = iter->node->value;

           iter->node = iter->node->next;
           if(!iter->node){
               iter->index++;
               while(iter->index < iter->map->size && !iter->map->buckets[iter->index]){
                   iter->index++;
               }

               if(iter->index < iter->map->size){
                   iter->node = iter->map->buckets[iter->index];
               }
           }

           return 1;
       }

       void hashmap_iterator_free(HashMapIterator *iter){
           free(iter);
       }
   ```

### That's it for the HashMap implementation in C!

**Date : 6 January 2025 11:09 PM**

### HashSet in C

- A hashset is a data structure that stores a set of unique elements.
- We will use it to store the set of valid addresses in the heap
- We can implement a HashSet using HashMap but I will like to keep a seperate Implementation.

### HashSet Implementation

1. **The HashSet Structure**
   - The hashset structure will store the array of linked lists.
   ```c
   typedef struct HashSet {
       HashSetNode **buckets;
       int size;
   } HashSet;
   ```
   - The `buckets` field is an array of linked lists.
   - The `size` field is the size of the array.
   - The HashSetNode structure will store the key.
   ```c
   typedef struct HashSetNode {
       uintptr_t key;
       struct HashSetNode *next;
   } HashSetNode;
   ```
2. **The HashSet Functions**

   1. HashSet Initialization

      - The `hashset_init()` function will initialize the hashset.

      ```c
      void hashset_init(HashSet *set){
          set->buckets = malloc(HASHSET_SIZE * sizeof(HashSetNode *));
          set->size = HASHSET_SIZE;

          for(int i = 0; i < HASHSET_SIZE; i++){
              set->buckets[i] = NULL;
          }
      }
      ```

   2. HashSet Insertion

      - The `hashset_insert()` function will insert a key into the hashset.
      - We will use the same hash function as the hashmap.

      ```c
      void hashset_insert(HashSet *set, uintptr_t key){
          if(hashset_lookup(set, key)) return;
          uintptr_t index = hash(key, set->size);

          HashSetNode *node = malloc(sizeof(HashSetNode));
          node->key = key;
          node->next = set->buckets[index];
          set->buckets[index] = node;
      }
      ```

   3. HashSet Lookup

      - The `hashset_lookup()` function will lookup if a key is present in the hashset.

      ```c
      int hashset_lookup(HashSet *set, uintptr_t key){
          uintptr_t index = hash(key, set->size);

          HashSetNode *node = set->buckets[index];
          while(node){
              if(node->key == key){
                  return 1;
              }
              node = node->next;
          }

          return 0;
      }
      ```

   4. HashSet Deletion

      - The `hashset_delete()` function will delete a key from the hashset.

      ```c
      void hashset_delete(HashSet *set, uintptr_t key){
          uintptr_t index = hash(key, set->size);

          HashSetNode *node = set->buckets[index];
          HashSetNode *prev = NULL;

          while(node){
              if(node->key == key){
                  if(prev){
                      prev->next = node->next;
                  } else {
                      set->buckets[index] = node->next;
                  }
                  free(node);
                  return;
              }
              prev = node;
              node = node->next;
          }
      }
      ```

   5. HashSet Free

      - The `hashset_free()` function will free the hashset.

      ```c
      void hashset_free(HashSet *set){
          for(int i = 0; i < set->size; i++){
              HashSetNode *node = set->buckets[i
              while(node){
                  HashSetNode *temp = node;
                  node = node->next;
                  free(temp);
              }
          }

          free(set->buckets);
          set->buckets = NULL;
          set->size = 0;
      }
      ```

   6. HashSet Iterator

      - The `hashset_iterator_create()` function will create an iterator for the hashset.

      ```c
      HashSetIterator *hashset_iterator_create(HashSet *set){
          HashSetIterator *iter = malloc(sizeof(HashSetIterator));
          iter->set = set;
          iter->index = 0;
          iter->node = NULL;

          while(iter->index < set->size && !set->buckets[iter->index]){
              iter->index++;
          }

          if(iter->index < set->size){
              iter->node = set->buckets[iter->index];
          }

          return iter;
      }

      int hashset_iterator_has_next(HashSetIterator *iter){
          return iter->node != NULL;
      }

      uintptr_t hashset_iterator_next(HashSetIterator *iter){
          if(!hashset_iterator_has_next(iter)) return 0;

          uintptr_t key = iter->node->key;

          iter->node = iter->node->next;
          if(!iter->node){
              iter->index++;
              while(iter->index < iter->set->size && !iter->set->buckets[iter->index]){
                  iter->index++;
              }

              if(iter->index < iter->set->size){
                  iter->node = iter->set->buckets[iter->index];
              }
          }

          return key;
      }

      void hashset_iterator_free(HashSetIterator *iter){
          free(iter);
      }
      ```

### That's it for the HashSet implementation in C!

**TO DO** : Add comments in the HashMap and HashSet Implementation, and write tests for them.

**Date: 5 January 2025 12:34 PM**

- Done with adding tests and comments for HashMap and HashSet Implementation.

### Now I will start implementing the basic mark and sweep garbage collector in C!

**Date: 5 January 2025 10:44pm**

- I've implemented a basic mark and sweep garbage collector but sometimes i am getting varaible answers
- I really will have to look into it

**Date: 7 January 2025 10:11 am**

- So now that I have implemented the basic mark and sweep garbage collector, I will now start implementing the basic mark and compact garbage collector.

- Let's start learning about the mark and compact algorithm.

**Date: 7 January 2025 11:44 pm**

- Well finally the bug was fixed and the garbage collector is working fine now.
- Special Thanks to [Aditya Deshmukh](https://github.com/adityasd314) for helping me out with the bug.
- Well i will explain the bug and the solution now.

### The Bug

- The bug was that the `get_roots()` function was not scanning the stack properly.
- This is because there was always some pointer which was pointing to some garbage memory location.
- The reason for this is what I'm yet to find out.
- But the solution was quite simple:
- Old Implementation:

```c
    /* earlier we did this in the get_roots function */
    static uint8_t *__rsp;

    #define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))

    __READ_RSP()
    stack_bottom = __rsp;
```

- New Implementation:

```c
    /* In the gc_init function we will store the stack_bottom and use it in the get_roots function*/
    int *a = (int *)malloc(sizeof(int));
    gc.stack_bottom = &a;
    free(a);
```

- This is more reliable than the earlier approach which used inlibe assembly to get the stack pointer.

```
Higher Memory Address
+------------------+
|   Other stack    |
|    frames        |
+------------------+
|    &a            |  ← gc.stack_bottom points here
+------------------+
| gc_init's local  |
| variables        |
+------------------+
| Return address   |
| and saved regs   |
+------------------+  ← gc.stack_top points here (__builtin_frame_address(1))
|   Caller's       |
|   stack frame    |
+------------------+
Lower Memory Address
```

- Now that the garbage collector is working fine, I will start implementing the mark and compact garbage collector.

**Date: 15 January 2025 07:53 pm**

- For the past few days I have been reading about the mark and compact garbage collector.
- References:
  1. [Wikipedia Article](https://en.wikipedia.org/wiki/Mark-compact_algorithm)
  2. [Presentation](https://www.cs.tau.ac.il//~maon/teaching/2014-2015/seminar/seminar1415a-lec2-mark-sweep-mark-compact.pdf)

### Why Mark and Compact?

- To answer this we need to understand the problem with the mark and sweep algorithm.
- The mark and sweep algorithm has a problem of fragmentation.
- When objects are freed from memory, the memory can be scattered, this can lead to fragmentation.
- Fragmentation is the phenomenon where the memory is divided into small chunks, but the chunks are not contiguous.
  <img src="./References/Images/05- Fragmentation.png" width="400">

- The mark and compact algorithm solves this problem by compacting the live objects in memory.

### What Algorithm does Mark and Compact use?

- There are a lot of algorithms that can be used for the mark and compact garbage collector.

  1. Table based compaction
  2. LISP2
  3. Compressor

- we will use the **LISP2** algorithm for the mark and compact garbage collector.

**Date: 16 January 2025 10:02 PM**

### How does the LISP2 Algorithm work?

- LISP2 is arguably the fastest compaction algorithm.
- It is a sliding collector algorithm.
- The LISP2 algorithm uses three different passes over the heap. In addition the heap objects must have a seperate forwarding pointer slot that is not used outside of the garbage collector.
- After the standard marking the Algorithm proceeds in the following three passes.

  1. Compute the forwarding location for live objects.
     - Keep track of a free and live pointer and initialize both to the start of heap.
     - If the live pointer points to a live object, update that object's forwarding pointer to the current free pointer and - increment the free pointer according to the object's size.
     - Move the live pointer to the next object
     - End when the live pointer reaches the end of heap.
  2. Update all pointers
     - For each live object, update its pointers according to the forwarding pointers of the objects they point to.
  3. Move objects
     - For each live object, move its data to its forwarding location.

- **this algorithm is O(n) on the size of the heap**

### Let's start implementing the mark and compact garbage collector

- The reference for this is the presentation I mentioned earlier.
- Basically the `gc_compact()` function will do the following:
  1. Compute the forwarding location for live objects.
  2. Update all pointers
  3. Move objects
- It will look like this:

```c
void gc_compact(HashMap *roots){
    compute_locations();
    update_references(roots);
    relocate();
}
```

- The `compute_locations()` function will compute the forwarding location for live objects.
- The `update_references()` function will update all the pointers.
- The `relocate()` function will move the objects.
- Also the metadata structure will now have a 'forwarding_address' field.

**Date: 17 January 2025 12:57 AM**

- Finally done with the basic implementation of the mark and compact garbage collector.
- I will explain the implementation tommorrow. I am really tired now.


**Date: 21 May 2025 2:21 PM**

So, it was a long time since I wrote the last log. I was busy with my exams and other stuff. But now I am back to work on the garbage collector.

I will now start explaining the implementation of the mark and compact garbage collector.

### The Implementation

**compute_locations()**

- The `compute_locations()` function will compute the forwarding location for live objects.
- The forwarding location is the new location in the memory where the object will be moved.

```c
void compute_locations(){
    MetaData *live = gc.list_head;
    MetaData *free = gc.list_head;

    while(live){
        if(live->marked){
            live->forwarding_address = free->address;
            free = free->next;
        }

        live = live->next;
    }
}
```

- Two pointers: `live` and `free` are initialized to the head of linked list that tracks all the metadata
- The list is traversed and if an object is marked, a forwarding address is assigned to it. This address is pointed by the `free` pointer.

- This can be explained like this:
    - Assume three blocks with middle one being garbage.
    - The forwarding address of the first block will be its address itself, because the `live` and `free` pointers are pointing to the same location
    - both the pointers are incremented and now point to the second block
    - because the second block is not marked the `live` pointer is incremented but the free pointer is not
    - for the third block now the **second block** will be the forwarding address

**Note** : here `live` and `free` point to metadata blocks, which contain the address, however for explanation purposes i have considered them to point to actual objects

**update_references**

- The `update_references()` function will update all the pointers in the heap.
- It processes both the roots and internal pointers of the allocated objects.
- again here we use the `duck test` to check if a memory location is a pointer or not.

```c

void update_references(HashMap *roots){
    HashMapIterator *iterator = hashmap_iterator_create(roots);
    uintptr_t *key;
    uintptr_t *value;

    while(hashmap_iterator_has_next(iterator)){
        hashmap_iterator_next(iterator, &key, &value);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, value);
        if(metadata){
            uintptr_t *new_address = (uintptr_t *)metadata->forwarding_address;
            if(new_address){
                *key = (uintptr_t)new_address;
            }
        }
    }

    MetaData *temp = gc.list_head;

    while(temp){
        uint8_t *start = temp->address;
        uint8_t *end = temp->address + temp->size;

        while(start < end){
            uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;
            MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
            if(metadata){
                uintptr_t *new_address = (uintptr_t *)metadata->forwarding_address;
                if(new_address){
                    *(uintptr_t *)start = (uintptr_t)new_address;
                }
            }
            
            start += sizeof(uintptr_t);
        }

        temp = temp->next;
    }


    hashmap_iterator_free(iterator);
}
```
- We iterate over the roots, and for each root, we get the metadata of the object.
- We check if a forwarding address is present, if it is, we update the root to point to the new address.

- After updating the roots, we traverse each object in the heap. (via the metadata linked list)
- for each object, we interpret it as sequence of pointers, scanning it from the start to end
- each assumed pointer is checked to see if it is a valid pointer
- if it is a valid pointer, we get its metadata and check if it has a forwarding address
- if it does, we update the pointer to point to the new address

**relocate()**

- The `relocate()` function physically moves the objects to their new locations.


```c
void relocate(){
    MetaData *temp = gc.list_head;
    int total_garbage = 0;

    while(temp){
        if(temp->marked){
            uintptr_t *destination = (uintptr_t *)temp->forwarding_address;
            MetaData *destination_metadata = (MetaData *)hashmap_lookup(gc.metadata, destination);
            uint8_t *source = temp->address;
        
            memcpy(destination, source, temp->size);
            destination_metadata->size = temp->size;
            destination_metadata->marked = 1;
        }else{
            total_garbage ++;
        }

        temp = temp->next;
    }
    temp = gc.list_head;
    int total_live_objects = gc.total_allocated - total_garbage;

    for(int i = 0; i < total_live_objects; i++){
        temp = temp->next;
    }

    while(temp){
        temp->marked = 0;
        temp = temp->next;
    }
}
```
- we iterate the metadata list, and then for each marked object, its contents are copied to the forwarding address 
- now, we still have a problem. because the marked objects are copied to their forwarding addresses, we need to unmark their original locations
- here we use a quick workaround by measuring the `total_live_objects` and then iterating the metadata list to set the `marked` field to 0
- Now, because all the live objects are moved to the beginning of the heap, we will mark the first `total_live_objects` as `marked` and the rest as `unmarked`


This was my implementation of a mark compact garbage collector.
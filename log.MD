
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

**Date : 4 January 2025 02:50 PM**

### Let's take a look at the algorithm with an example

1. **The Traceable Object**
    - This will be the most important part of our implementation.
    - This will contain all the information about the object, which will be used by the garbage collector.
    - But for now for now we will just have a simple structure. This example is take from Dmitry Soshnikov's blog, with a few modifications.
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
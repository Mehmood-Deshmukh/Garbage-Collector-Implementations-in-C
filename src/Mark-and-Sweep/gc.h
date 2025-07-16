#ifndef GC_H /* Make sure this header file is included only once */
#define GC_H

#include "../HashSet-Implementation/hashset.h"
#include "../HashMap-Implementation/hashmap.h"
#include <stdint.h>
#include <stdlib.h>

/* 
 * This is a struct to "Store the metadata of the object".
 * Unlike, the metadata in Java, which stores object's class, lock information
 *  for synchronization, etc., source - https://www.geeksforgeeks.org/java/how-are-java-objects-stored-in-memory/
 * 
 * we simply store the size of the object and a marked flag. when a object is reachable,
 * the marked flag is set to 1, otherwise it is 0.
 * 
 */

typedef struct MetaData {
    int marked;
    size_t size;
} MetaData;

/* 
 * This is the main struct for the garbage collector.
 * It contains:
 * 
 * 1. Hashset *address: A set of all allocated addresses. When we allocate any memory, 
 * we insert the address into this set. It will further be useful to check if an address is valid or not.
 * 
 * 2. HashMap *metadata: A map of addresses to their metadata. This is used to store the metadata of the object
 * Now, we could have stored the metadata in the address itself, but that would require us to allocate
 * a block of (required size + sizeof(MetaData)) bytes, and  access the metadata by subtracting
 * sizeof(MetaData) from the address. source - https://github.com/sameerkavthekar/garbage-collector
 * However, this would make the code more complex and less readable. So we choose to use a hashmap
 * 
 * 3. void *stack_top: The top of the stack.
 * 4. void *stack_bottom: The bottom of the stack.
 * 
 * They will be used to find the roots of the garbage collector. We will scan the stack from the 
 * bottom to the top, and find all the addresses that are valid in the garbage collector's address set.
 */

typedef struct GC {
    HashSet *address;
    HashMap *metadata;
    void *stack_top;
    void *stack_bottom;
} GC;

/*
 * This is the global variable for the garbage collector. Why is it extern?
 * Let's first see what's the definition of extern:
 * The extern keyword is used to declare a variable that is defined in another file.
 * In this case, we are defining the gc variable in gc.c file, and we want
 * to use it in other files as well. So we declare it as extern in this header file. 
 * 
 * The usual practice is to collect extern declarations in a header file, Source - KnR
 * 
 */

extern GC gc;

/*  Function declarations for the garbage collector. */
void gc_init();
void *gc_malloc(size_t size);
void gc_run();
void gc_free(uintptr_t *address);
void gc_dump(char *message);


#endif /* GC_H */

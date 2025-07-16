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
 * Additions for mark and compact:
 * 
 * when implementing a mark and compact garbage collector, we need to store additional information
 * for example we need to store the address to which current object is forwarded during compaction.
 * Also as you'll see further are mark-compact algorithm requires us to maintain a linked-list of
 * objects (not really objects but metadata will suffice if we store addresses in them!)
 * 
 * so yeah, we need the following additions in the metadata block
 * 1. address: The address of the allocated memory block.
 * 2. forwarding_address: The address to which the object is forwarded during compaction.
 * 3. next: A pointer to the next metadata in a linked list for managing free blocks.
 * 
 * 
 */

typedef struct MetaData {
    int marked;
    size_t size;
    uintptr_t *address;
    uintptr_t *forwarding_address;
    struct MetaData *next;
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
 * 
 * 
 * Additions for mark and compact:
 * 
 * the mark and compact garbage collector requires us to maintain a linked list of metadata blocks
 * so that we can easily traverse the allocated blocks during calculation of forwarding addresses 
 * hence, we obviously need the head and tails of the linked list.
 * and also to free the blocks after compaction. I have used a work around to unmark the blocks as 
 * unvisited which needs the total number of objects allocated in the garbage collector.
 * 
 * so the additions are:
 * 1. Metadata *list_head : A pointer to the head of the linked list of metadata blocks.
 * 2. Metadata * list_tail : A pointer to the tail of the linked list of metadata blocks.
 * 3. int total_allocated : The total number of objects allocated in the garbage collector.
 */


typedef struct GC{
    HashSet *address;
    HashMap *metadata;
    void *stack_top;
    void *stack_bottom;
    MetaData *list_head;
    MetaData *list_tail;
    int total_allocated;
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
void gc_run();
void gc_dump(char *message);
void *gc_malloc(size_t size);
void gc_free(uintptr_t *address);

#endif /* GC_H */

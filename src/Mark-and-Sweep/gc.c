#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"

void print_hashset(HashSet *set);

GC gc;

void gc_init() {
    gc.stack_top = __builtin_frame_address(1);
    gc.address = malloc(sizeof(HashSet));
    gc.metadata = malloc(sizeof(HashMap));

    int *a = (int *)malloc(sizeof(int));
    gc.stack_bottom = &a;
    free(a);

    if(!gc.address || !gc.metadata){
        printf("Unable to allocate memory for gc initialization\n");
        exit(1);
    }

    hashset_init(gc.address);
    hashmap_init(gc.metadata);
}

HashSet *get_roots(){
    jmp_buf jb;
    setjmp(jb);

    HashSet *roots = malloc(sizeof(HashSet));
    if(!roots){
        printf("Unable to allocate memory for roots\n");
        exit(1);
    }
    hashset_init(roots);

    uint8_t *stack_bottom = (uint8_t *)gc.stack_bottom + sizeof(uintptr_t);
    uint8_t *stack_top = (uint8_t *)gc.stack_top;


    while(stack_bottom < stack_top){
        uintptr_t *address = (uintptr_t *)*(uintptr_t *)stack_bottom;
        if(((uintptr_t)address % sizeof(uintptr_t)) == 0){
            if(hashset_lookup(gc.address, address)){
                hashset_insert(roots, address);
            }
        }
        stack_bottom += sizeof(uintptr_t);
    }

    return roots;
}

/* 
 * About this function:
 *
 * This function returns the children of the object at the given address.
 * What are children?
 * - As we are implementing a "conservative" garbage collector, we consider
 * any "pointer-like" value in the memory as a pointer to another object.
 * So, the children of an object are the pointer-like values that point to
 * valid addresses in the garbage collector's address set.
 * 
 * 
 * This is our funny little "Duck Test" - if it looks like a duck, swims like a duck,
 * and quacks like a duck, then it is a duck.
 * 
 * In our case, if it looks like a pointer, and points to a valid address, 
 * then it is a pointer. reference: (http://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/)
 * 
 * - This is a conservative approach because it may include values that are not
 * actually pointers, but we treat them as such.
 * 
 * for example:
 *   int *ptr = malloc(sizeof(int));
 *   int a = (int) ptr;  This is a pointer-like value, but it is not a pointer.
 *   ptr = null
 * 
 * if we run our garbage collector after this code, it will consider `a` as a pointer
 * 
 * 
 * How it works:
 * 
 * 1. check if the address is valid and exists in the garbage collector's address set.
 * 2. get the metadata for the address from the hashmap.
 * 3. create a new HashSet to store the children.
 * 4. iterate over the memory block of the object at the given address.
 *    - Now, initially i thought that i need to keep a window of size of a pointer
 *      and move that window by one byte at a time. 
 *    - This seemed reasonable right? but C makes our life easier. pointers are always
 *      aligned to the size of a pointer, so we can just iterate over the memory
 *      block by the size of a pointer.
 *    - for each pointer-like value in the memory block, check if it is a valid address
 *      in the garbage collector's address set.
 *    - if it is, insert it into the children HashSet, else ignore it.
 *    - increment the start pointer by the size of a pointer.
 * 5. return the children HashSet.
 * 
 * Now, let's see the most confusing part, the scan:
 * - The starting point will be the address of the object.
 * - The end point will be the address of the object + size of the object.
 * - We will iterate over the memory block from start to end, checking each pointer-like value
 *   to see if it is a valid address in the garbage collector's address set.
 * - Now, if you read the code you'll find a lot of typecasting and pointer arithmetic,
 *   but don't worry, i'll try to explain it in a simple way.
 * 
 * 1. start = address;
 * 2. end = (uintptr_t *)((uint8_t *)address + metadata->size);
 *   - Here, we are calculating the end address by adding the size of the object
 *    to the address of the object. Why did we cast address to (uint8_t *)?
 *   - The answer lies in "pointer arithmetic". In c pointers are incremented by
 *     the size of the type they point to.
 *     for example, if you have a pointer to an int, and you increment it by 1,
 *     it will actually increment by sizeof(int) bytes.
 *   - so if we simply perform address + metadata->size, it will not give us the
 *    correct end address, because it will increment by the size of a pointer * metadata->size.
 *   - So, we cast the address to (uint8_t *) which is a byte pointer, and then increment it
 *     by metadata->size bytes to get the correct increment of metadata->size * 1 bytes.
 * 
 * 3. While start < end:
 *   - We will iterate over the memory block from start to end, checking each pointer-like  
 *     value to see if it is a valid address in the garbage collector's address set.
 *   - uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;
 *   - Here, we are dereferencing the start pointer to get the value at that address,
 *     and then casting it to a uintptr_t pointer. This is because we are treating the
 *     value at that address as a pointer-like value.
 *   - if(((uintptr_t) address % sizeof(uintptr_t)) == 0){
 *   - This checks if the address is aligned to the size of a pointer. If it is,
 *     we consider it as a valid pointer-like value.
 *   - if(hashset_lookup(gc.address, address)){
 *   - This checks if the address is a valid address in the garbage collector's address set.
 *   - if it is, we insert it into the children HashSet.
 *
 *  Wooho! We have our children HashSet now!
 *
 */

HashSet *get_children(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return NULL; /* return if  address is NULL or not in the address set */

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(!metadata) return NULL;

    HashSet *children = malloc(sizeof(HashSet));
    if(!children){
        printf("Unable to allocate memory for children\n");
        exit(1);
    }
    hashset_init(children);

    uintptr_t *start = address;
    uintptr_t *end = (uintptr_t *)((uint8_t *)address + metadata->size); /* casting it to (uint8_t *) to increment by bytes */

    while(start < end){
        uintptr_t *address = (uintptr_t *)*start;

        if(((uintptr_t) address % sizeof(uintptr_t)) == 0){ /* check if the address is aligned to the size of a pointer */
            if(hashset_lookup(gc.address, address)){
                hashset_insert(children, address); /* if it points to a valid address, insert it into the children HashSet */
            }
        }

        start ++; /* This will increment the start pointer by the sizeof(uintptr_t) bytes */
    }
    return children;
}

void gc_mark_helper(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(!metadata || metadata->marked) return;

    metadata->marked = 1;

    HashSet *children = get_children(address);
    if(!children) return;
    HashSetIterator *iterator = hashset_iterator_create(children);
    if(!iterator){
        hashset_free(children);
        free(children);
        return;
    }

    while(hashset_iterator_has_next(iterator)){
        gc_mark_helper(hashset_iterator_next(iterator));
    }

    hashset_free(children);
    free(children);
    hashset_iterator_free(iterator);
}

void gc_mark(HashSet *roots){
    if(!roots) return;

    HashSetIterator *iterator = hashset_iterator_create(roots);
    if(!iterator) return;

    while(hashset_iterator_has_next(iterator)){
        uintptr_t *address = hashset_iterator_next(iterator);
        gc_mark_helper(address);
    }

    hashset_iterator_free(iterator);
}

void gc_sweep(){
    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    if(!iterator) return;

    while(hashset_iterator_has_next(iterator)){
        uintptr_t *address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        if(!metadata) continue;

        if(metadata->marked == 0){
            gc_free(address);
        } else {
            metadata->marked = 0;
        }
    }

    hashset_iterator_free(iterator);
}

void gc_run(){
    HashSet *roots = get_roots();
    if(!roots) return;

    gc_mark(roots);
    gc_sweep();

    hashset_free(roots);
    free(roots);
}

void gc_dump(char *message){
    printf("%s\n\n", message);
    printf("{\n");

    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    if(!iterator) return;

    int count = 0;
    while(hashset_iterator_has_next(iterator)){
        count++;
        uintptr_t *address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        if(!metadata) continue;
        printf("\t%p : {marked: %d, size: %zu},\n", address, metadata->marked, metadata->size);
    }
    printf("\n\nTotal Allocated: %d\n", count);
    printf("}\n");

    hashset_iterator_free(iterator);
}

void *gc_malloc(size_t size){
    if(size == 0) return NULL;

    void *address = (void *)calloc(1, size);
    if(!address){
        printf("Unable to allocate memory for size %zu\n", size);
        exit(1);
    }

    MetaData *metadata = (MetaData *)malloc(sizeof(MetaData));
    if(!metadata){
        printf("Unable to allocate memory for metadata\n");
        exit(1);
    }

    metadata->marked = 0;
    metadata->size = size;

    hashset_insert(gc.address, address);
    hashmap_insert(gc.metadata, address, (uintptr_t *)metadata);

    return address;
}

void gc_free(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(metadata) free(metadata);

    hashset_delete(gc.address, address);
    hashmap_delete(gc.metadata, address);
    free(address);
}


void print_hashset(HashSet *set){
    printf("====================\n");
    HashSetIterator *iterator = hashset_iterator_create(set);
    while(hashset_iterator_has_next(iterator)){
        uintptr_t *address = hashset_iterator_next(iterator);
        printf("%p\n", address);
    }
    hashset_iterator_free(iterator);
    printf("====================\n");
}


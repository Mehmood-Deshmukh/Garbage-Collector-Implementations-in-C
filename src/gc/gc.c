#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"

void gc_init() {
    gc.address = malloc(sizeof(HashSet));
    gc.metadata = malloc(sizeof(HashMap));
    gc.stack_top = __builtin_frame_address(1);
    hashset_init(gc.address);
    hashmap_init(gc.metadata);
}

HashSet *get_roots(){
    jmp_buf jb;
    setjmp(jb);
    
    __READ_RSP();

    HashSet *roots = malloc(sizeof(HashSet));
    hashset_init(roots);


    uintptr_t *stack_bottom = (uintptr_t *)__rsp;
    uintptr_t *stack_top = (uintptr_t *)gc.stack_top;

    while(stack_bottom < stack_top){
        uintptr_t address = (uintptr_t)*(uintptr_t *)stack_bottom;
        if(hashset_lookup(gc.address, address)){
            printf("%p is pointing to %p\n", stack_bottom, (void *)address);
            hashset_insert(roots, address);
        }
        stack_bottom++;
    }
    return roots;
}

HashSet *get_children(uintptr_t address){
    HashSet *children = malloc(sizeof(HashSet));
    hashset_init(children);
    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);


    uintptr_t *start = (uintptr_t *)address;
    uintptr_t *end = (uintptr_t *)(address + metadata->size);

    while(start < end){
        if(hashset_lookup(gc.address, *start)){
            hashset_insert(children, *start);
        }
        start++;
    }
    return children;
}

void gc_mark_helper(uintptr_t address){
    if(!hashset_lookup(gc.address, address)) return;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(metadata->marked == 1) return;

    metadata->marked = 1;

    HashSet *children = get_children(address);
    HashSetIterator *iterator = hashset_iterator_create(children);

    while(hashset_iterator_has_next(iterator)){
        gc_mark_helper(hashset_iterator_next(iterator));
    }
}

void gc_mark(HashSet *roots){
    HashSetIterator *iterator = hashset_iterator_create(roots);

    while(hashset_iterator_has_next(iterator)){
        uintptr_t address = hashset_iterator_next(iterator);
        gc_mark_helper(address);
    }
}

void gc_sweep(){
    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    while(hashset_iterator_has_next(iterator)){
        uintptr_t address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        if(metadata->marked == 0){
            hashset_delete(gc.address, address);
            hashmap_delete(gc.metadata, address);
            free((uintptr_t *)address);
        } else {
            metadata->marked = 0;
        }
    }
}

void gc_run(){
    HashSet *roots = get_roots();
    gc_mark(roots);
    gc_sweep();
}

void gc_dump(char *message){
    printf("%s\n\n", message);
    printf("{\n");
    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    int count = 0;
    while(hashset_iterator_has_next(iterator)){
        count++;
        uintptr_t address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        printf("\t%p : {marked: %d, size: %zu},\n", (void *)address, metadata->marked, metadata->size);
    }
    printf("Total Allocated: %d\n", count);
    printf("}\n");
}

void *gc_malloc(size_t size){
    void *address = malloc(size);
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

    hashset_insert(gc.address, (uintptr_t)address);
    hashmap_insert(gc.metadata, (uintptr_t)address, (uintptr_t)metadata);

    return address;
}

void gc_free(uintptr_t *address){
    hashset_delete(gc.address, (uintptr_t)address);
    hashmap_delete(gc.metadata, (uintptr_t)address);
    free(address);
}

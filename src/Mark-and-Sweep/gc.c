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

HashSet *get_children(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return NULL;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(!metadata) return NULL;

    HashSet *children = malloc(sizeof(HashSet));
    if(!children){
        printf("Unable to allocate memory for children\n");
        exit(1);
    }
    hashset_init(children);

    uint8_t *start = (uint8_t *)address;
    uint8_t *end = (uint8_t *)((uint8_t *)address + metadata->size);

    while(start < end){
        uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;

        if(((uintptr_t) address % sizeof(uintptr_t)) == 0){
            if(hashset_lookup(gc.address, address)){
                hashset_insert(children, address);
            }
        }

        start += sizeof(uintptr_t);
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
    gc_dump("After Marking");
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


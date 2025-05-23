#ifndef GC_H
#define GC_H

#include "../../src/HashSet-Implementation/hashset.h"
#include "../../src/HashMap-Implementation/hashmap.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct MetaData {
    int marked;
    size_t size;
    uint8_t *address;
    uint8_t *forwarding_address;
    struct MetaData *next;
} MetaData;

typedef struct GC{
    HashSet *address;
    HashMap *metadata;
    void *stack_top;
    void *stack_bottom;
    MetaData *list_head;
    MetaData *list_tail;
    int total_allocated;
} GC;

extern GC gc;

void gc_init();
void gc_run();
void gc_dump(char *message);
void *gc_malloc(size_t size);
void gc_free(uintptr_t *address);

#endif /* GC_H */

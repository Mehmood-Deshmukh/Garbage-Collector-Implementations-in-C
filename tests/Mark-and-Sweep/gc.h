#ifndef GC_H
#define GC_H

#include "../../src/HashSet-Implementation/hashset.h"
#include "../../src/HashMap-Implementation/hashmap.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct MetaData {
    int marked;
    size_t size;
} MetaData;

typedef struct GC{
    HashSet *address;
    HashMap *metadata;
    void *stack_top;
    void *stack_bottom;
} GC;

extern GC gc;

void gc_init();
void gc_run();
void gc_dump(char *message);
void *gc_malloc(size_t size);
void gc_free(uintptr_t *address);

#endif /* GC_H */

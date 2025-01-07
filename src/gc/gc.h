#ifndef GC_H
#define GC_H

#include "../HashSet Implementation/hashset.h"
#include "../HashMap Implementation/hashmap.h"
#include <stdint.h>
#include <stdlib.h>


static uint8_t *__rsp;

#define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))

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

static GC gc;

void gc_init();
void gc_run();
void gc_dump(char *message);
void *gc_malloc(size_t size);
void gc_free(uintptr_t *address);

#endif /* GC_H */

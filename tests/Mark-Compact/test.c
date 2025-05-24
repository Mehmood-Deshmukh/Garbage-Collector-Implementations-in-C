#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

void test_initialization() {
    gc_init();

    if(gc.address == NULL || gc.metadata == NULL) {
        printf("Initialization failed: address or metadata is NULL\n");
        exit(1);
    }else {
        printf("Initialization successful: address and metadata are not NULL\n");
    }

    if(gc.list_head == NULL && gc.list_tail == NULL) {
        printf("Initialization successful: list_head and list_tail are NULL\n");
    } else {
        printf("Initialization failed: list_head or list_tail is not NULL\n");
    }

    if(gc.total_allocated == 0) {
        printf("Initialization successful: total_allocated is 0\n");
    } else {
        printf("Initialization failed: total_allocated is not 0\n");
    }

    if(gc.stack_top != NULL && gc.stack_bottom != NULL) {
        printf("Initialization successful: stack_top and stack_bottom are not NULL\n");
    } else {
        printf("Initialization failed: stack_top or stack_bottom is NULL\n");
    }
}

void test_gc_malloc() {
    int *ptr = (int *)gc_malloc(sizeof(int));
    if(ptr == NULL) {
        printf("gc_malloc failed: returned NULL\n");
        exit(1);
    } else {
        printf("gc_malloc successful: returned non-NULL pointer\n");
    }

    *ptr = 42;
    printf("Value at allocated memory: %d\n", *ptr);

    if(gc.total_allocated == 0) {
        printf("gc_malloc failed: total_allocated is not updated\n");
    } else {
        printf("gc_malloc successful: total_allocated is updated\n");
    }

    if(gc.list_head == NULL && gc.list_tail == NULL) {
        printf("gc_malloc failed: list_head and list_tail are not updated\n");
    } else {
        printf("gc_malloc successful: list_head and list_tail are updated\n");
    }

    if(hashset_lookup(gc.address, (uintptr_t *)ptr) == 0) {
        printf("gc_malloc failed: address not found in hashset\n");
    } else {
        printf("gc_malloc successful: address found in hashset\n");
    }

    if(hashmap_lookup(gc.metadata, (uintptr_t *)ptr) == 0) {
        printf("gc_malloc failed: metadata not found in hashmap\n");
    } else {
        printf("gc_malloc successful: metadata found in hashmap\n");
    }


    gc_free(ptr);
}

void test_gc_free() {
    int *ptr = (int *)gc_malloc(sizeof(int));
    if(ptr == NULL) {
        printf("gc_malloc failed: returned NULL\n");
        exit(1);
    }

    gc_free((uintptr_t *)ptr);

    if(hashset_lookup(gc.address, (uintptr_t *)ptr) != 0) {
        printf("gc_free failed: address still found in hashset\n");
    } else {
        printf("gc_free successful: address not found in hashset\n");
    }

    if(hashmap_lookup(gc.metadata, (uintptr_t *)ptr) != 0) {
        printf("gc_free failed: metadata still found in hashmap\n");
    } else {
        printf("gc_free successful: metadata not found in hashmap\n");
    }

    
}

int main(){
  test_initialization();
  test_gc_malloc();
  return 0;
}

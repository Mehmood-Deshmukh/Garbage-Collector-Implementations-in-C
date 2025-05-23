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

    gc_free((uintptr_t *)ptr);
    printf("Memory freed successfully\n");
}

int main(){
  test_initialization();
  test_gc_malloc();
  return 0;
}

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include "gc.h"

void print_test_result(char *test_name, int result);
void assert_equal(uintptr_t expected, uintptr_t actual, char *error_message);
void test_gc_init();
void test_gc_malloc();
void test_gc_free();
void test_gc_mark_and_sweep();
void test_gc_run();

typedef struct TestObj {
    int value;
    struct TestObj* next;
} TestObj;

int main(){
    printf("Running tests...\n");
    printf("Test 1: Testing GC Initialization\n");
    test_gc_init();
    printf("Test 2: Testing GC Malloc\n");
    test_gc_malloc();
    printf("Test 3: Testing GC Free\n");
    test_gc_free();
    printf("Test 4: Testing Mark and Sweep\n");
    test_gc_mark_and_sweep();
    printf("Test 5: Testing GC Run\n");
    test_gc_run();
    printf("All tests passed!\n");
    return 0;
}

void print_test_result(char *test_name, int result){
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
}

void assert_equal(uintptr_t expected, uintptr_t actual, char *error_message){
    if(expected != actual){
        printf("Assertion failed: %s\n", error_message);
        printf("Expected: %lu, Actual: %lu\n", expected, actual);
        exit(1);
    }
}

void test_gc_init(){
    gc_init();
    assert_equal(0, gc.total_allocated, "Total allocated should be 0");
    assert_equal((uintptr_t)NULL, (uintptr_t)gc.list_head, "List head should be NULL");
    assert_equal((uintptr_t)NULL, (uintptr_t)gc.list_tail, "List tail should be NULL");
    print_test_result("Test 1: Testing GC Initialization", 1);
}

void test_gc_malloc(){
    gc_init();
    int *ptr = (int *)gc_malloc(sizeof(int));
    assert_equal(1, ptr != NULL, "Malloc should return non-NULL");
    assert_equal(1, hashset_lookup(gc.address, (uintptr_t *)ptr), "Pointer should be tracked");
    assert_equal(1, hashmap_lookup(gc.metadata, (uintptr_t *)ptr) != NULL, "Metadata should exist");
    assert_equal(1, gc.total_allocated > 0, "Total allocated should increase");
    
    void *null_ptr = gc_malloc(0);
    assert_equal((uintptr_t)NULL, (uintptr_t)null_ptr, "Malloc with size 0 should return NULL");
    
    print_test_result("Test 2: Testing GC Malloc", 1);
}

void test_gc_free(){
    gc_init();
    int *ptr = (int *)gc_malloc(sizeof(int));
    int initial_allocated = gc.total_allocated;
    
    gc_free((uintptr_t *)ptr);
    assert_equal(0, hashset_lookup(gc.address, (uintptr_t *)ptr), "Freed pointer should not be tracked");
    assert_equal((uintptr_t)NULL, (uintptr_t)hashmap_lookup(gc.metadata, (uintptr_t *)ptr), "Metadata should be removed");
    assert_equal(1, gc.total_allocated < initial_allocated, "Total allocated should decrease");
    
    gc_free(NULL);
    print_test_result("Test 3: Testing GC Free", 1);
}

void test_gc_mark_and_sweep(){
    gc_init();
    
    TestObj *obj1 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *obj2 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *obj3 = (TestObj *)gc_malloc(sizeof(TestObj));
    
    obj1->next = obj2;
    obj2->next = NULL;
    
    HashMap *roots = malloc(sizeof(HashMap));
    hashmap_init(roots);
    uintptr_t fake_stack_addr = (uintptr_t)&obj1;
    hashmap_insert(roots, (uintptr_t *)fake_stack_addr, (uintptr_t *)obj1);
    
    gc_mark(roots);
    
    MetaData *metadata1 = (MetaData *)hashmap_lookup(gc.metadata, (uintptr_t *)obj1);
    MetaData *metadata2 = (MetaData *)hashmap_lookup(gc.metadata, (uintptr_t *)obj2);
    MetaData *metadata3 = (MetaData *)hashmap_lookup(gc.metadata, (uintptr_t *)obj3);
    
    assert_equal(1, metadata1->marked, "Root object should be marked");
    assert_equal(1, metadata2->marked, "Referenced object should be marked");
    assert_equal(0, metadata3->marked, "Unreferenced object should not be marked");
    
    int initial_count = 0;
    MetaData *temp = gc.list_head;
    while(temp){
        initial_count++;
        temp = temp->next;
    }
    
    gc_sweep();
    
    int after_sweep_count = 0;
    temp = gc.list_head;
    while(temp){
        after_sweep_count++;
        temp = temp->next;
    }
    
    assert_equal(1, after_sweep_count == initial_count - 1, "Sweep should remove unmarked objects");
    assert_equal(0, hashset_lookup(gc.address, (uintptr_t *)obj3), "Unmarked object should be removed");
    
    hashmap_free(roots);
    free(roots);
    print_test_result("Test 4: Testing Mark and Sweep", 1);
}

void test_gc_run(){
    gc_init();
    
    TestObj *obj1 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *obj2 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *unreachable = (TestObj *)gc_malloc(sizeof(TestObj));
    unreachable = NULL;
    
    obj1->next = obj2;
    obj2->next = NULL;
    
    int initial_count = 0;
    MetaData *temp = gc.list_head;
    while(temp){
        initial_count++;
        temp = temp->next;
    }
    
    gc_run();
    
    int after_gc_count = 0;
    temp = gc.list_head;
    while(temp){
        after_gc_count++;
        temp = temp->next;
    }
    
    assert_equal(1, after_gc_count == initial_count - 1, "GC should collect unreachable objects");
    assert_equal(0, hashset_lookup(gc.address, (uintptr_t *)unreachable), "Unreachable object should be collected");
    assert_equal(1, hashset_lookup(gc.address, (uintptr_t *)obj1), "Reachable object should remain");
    assert_equal(1, hashset_lookup(gc.address, (uintptr_t *)obj2), "Referenced object should remain");
    
    print_test_result("Test 5: Testing GC Run", 1);
}
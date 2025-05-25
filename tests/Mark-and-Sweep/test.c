#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include "gc.h"

void print_test_result(char *test_name, int result);
void assert_equal(uintptr_t expected, uintptr_t actual, char *error_message);
void test_gc_init();
void test_gc_malloc();
void test_gc_free();
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
    printf("Test 4: Testing GC Run\n");
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
    assert_equal(1, gc.address != NULL, "Address set should be initialized");
    assert_equal(1, gc.metadata != NULL, "Metadata map should be initialized");
    assert_equal(1, gc.stack_top != NULL, "Stack top should be set");
    assert_equal(1, gc.stack_bottom != NULL, "Stack bottom should be set");
    print_test_result("Test 1: Testing GC Initialization", 1);
}

void test_gc_malloc(){
    gc_init();
    
    int *ptr = (int *)gc_malloc(sizeof(int));
    assert_equal(1, ptr != NULL, "Malloc should return non-NULL");
    assert_equal(1, hashset_lookup(gc.address, (uintptr_t *)ptr), "Pointer should be tracked in address set");
    
    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, (uintptr_t *)ptr);
    assert_equal(1, metadata != NULL, "Metadata should exist");
    assert_equal(sizeof(int), metadata->size, "Metadata size should be correct");
    assert_equal(0, metadata->marked, "Object should initially be unmarked");
    
    void *null_ptr = gc_malloc(0);
    assert_equal((uintptr_t)NULL, (uintptr_t)null_ptr, "Malloc with size 0 should return NULL");
    
    print_test_result("Test 2: Testing GC Malloc", 1);
}

void test_gc_free(){
    gc_init();
    
    int *ptr = (int *)gc_malloc(sizeof(int));
    assert_equal(1, hashset_lookup(gc.address, (uintptr_t *)ptr), "Pointer should be tracked before free");
    
    gc_free((uintptr_t *)ptr);
    assert_equal(0, hashset_lookup(gc.address, (uintptr_t *)ptr), "Freed pointer should not be tracked");
    assert_equal((uintptr_t)NULL, (uintptr_t)hashmap_lookup(gc.metadata, (uintptr_t *)ptr), "Metadata should be removed");
    
    gc_free(NULL);
    
    int *untracked = (int *)malloc(sizeof(int));
    gc_free((uintptr_t *)untracked);
    free(untracked);
    
    print_test_result("Test 3: Testing GC Free", 1);
}

TestObj *getTestObjs(){
    TestObj *obj1 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *obj2 = (TestObj *)gc_malloc(sizeof(TestObj));
    TestObj *obj3 = (TestObj *)gc_malloc(sizeof(TestObj));
    
    obj1->value = 1;
    obj1->next = obj2;
    obj2->value = 2;
    obj2->next = obj3;
    obj3->value = 3;
    obj3->next = NULL;

    return obj1; 
}

void test_gc_run(){
    gc_init();
    
    TestObj *obj1 = getTestObjs();
    
    
    int initial_obj1_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1);
    int initial_obj2_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1->next);
    int initial_obj3_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1->next->next);
    
    
    assert_equal(1, initial_obj1_tracked, "obj1 should be tracked before GC");
    assert_equal(1, initial_obj2_tracked, "obj2 should be tracked before GC");
    assert_equal(1, initial_obj3_tracked, "obj3 should be tracked before GC");
    
    obj1->next->next = NULL;
    
    gc_run();
    
    int after_obj1_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1);
    int after_obj2_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1->next);
    int after_obj3_tracked = hashset_lookup(gc.address, (uintptr_t *)obj1->next->next);
    
    assert_equal(1, after_obj1_tracked, "obj1 should remain after GC (stack reference)");
    assert_equal(1, after_obj2_tracked, "obj2 should remain after GC (referenced by obj1)");
    assert_equal(0, after_obj3_tracked, "obj3 should be collected (unreachable)");
    
    print_test_result("Test 4: Testing GC Run", 1);
}

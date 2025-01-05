#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include "hashset.h"

uintptr_t hash(uintptr_t key, int size);

void print_test_result(char *test_name, int result);
void assert_equal(uintptr_t expected, uintptr_t actual, char *error_message);
void test_init();
void test_insert_and_lookup();
void test_multiple_inserts();
void test_update();
void test_iterator();
void test_collision_handling();
void test_stress();

int main(){
    printf("Running tests...\n");
    printf("Test 1: Testing Initialization\n");
    test_init();
    printf("Test 2: Testing Insert and Lookup\n");
    test_insert_and_lookup();
    printf("Test 3: Testing Multiple Inserts\n");
    test_multiple_inserts();
    printf("Test 4: Testing Update\n");
    test_update();
    printf("Test 5: Testing Iterator\n");
    test_iterator();
    printf("Test 6: Testing Collision Handling\n");
    test_collision_handling();
    printf("Test 7: Testing Stress\n");
    test_stress();
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

void test_init(){
    HashSet set;
    hashset_init(&set);
    assert_equal(HASHSET_SIZE, set.size, "Size should be initialized");
    for(int i = 0; i < HASHSET_SIZE; i++){
        assert_equal((uintptr_t )NULL, (uintptr_t )set.buckets[i], "Buckets should be initialized");
    }
    print_test_result("Test 1: Testing Initialization", 1); 
}

void test_insert_and_lookup(){
    HashSet set;
    hashset_init(&set);
    uintptr_t key = 0x7ff000000000;
    hashset_insert(&set, key);
    int result = hashset_lookup(&set, key);
    assert_equal(1, result, "Key should be found");
    print_test_result("Test 2: Testing Insert and Lookup", 1);
}

void test_multiple_inserts(){
    HashSet set;
    hashset_init(&set);
    int n = 100;
    uintptr_t base_address = 0x7ff000000000;
    for(int i = 0; i < n; i++){
        uintptr_t key = base_address + i;
        hashset_insert(&set, key);
    }
    for(int i = 0; i < n; i++){
        uintptr_t key = base_address + i;
        int result = hashset_lookup(&set, key);
        assert_equal(1, result, "Key should be found");
    }

    print_test_result("Test 3: Testing Multiple Inserts", 1);
}

void test_update(){
    HashSet set;
    hashset_init(&set);
    uintptr_t key = 0x7ff000000000;
    hashset_insert(&set, key);
    int result = hashset_lookup(&set, key);
    assert_equal(1, result, "Key should be found");

    hashset_insert(&set, key);
    result = hashset_lookup(&set, key);
    assert_equal(1, result, "Key should be found");

    print_test_result("Test 4: Testing Update", 1);
}

void test_iterator(){
    HashSet set;
    hashset_init(&set);
    int n = 100;
    uintptr_t base_address = 0x7ff000000000;
    for(int i = 0; i < n; i++){
        uintptr_t key = base_address + i;
        hashset_insert(&set, key);
    }

    HashSetIterator *iter = hashset_iterator_create(&set);
    int count = 0;
    while(hashset_iterator_has_next(iter)){
        uintptr_t key = hashset_iterator_next(iter);
        int result = hashset_lookup(&set, key);
        assert_equal(1, result, "Key should be found");
        count++;
    }

    hashset_iterator_free(iter);
    print_test_result("Test 5: Testing Iterator", count == n);
}

void test_collision_handling(){
    uintptr_t base = 0x7ff000000000;
    uintptr_t  base_hash = hash(base, HASHSET_SIZE);
    int found = 0;
    int i;
    for(i = 1; i < 10000; i++){
        uintptr_t key = base + i;
        uintptr_t hash_value = hash(key, HASHSET_SIZE);
        if(hash_value == base_hash){
            found = 1;
            break;
        }
    }
    if(!found){
        printf("No collision found\n");
        return;
    }

    HashSet set;
    hashset_init(&set);
    uintptr_t key1 = base;
    uintptr_t key2 = base + i;

    hashset_insert(&set, key1);
    hashset_insert(&set, key2);

    int result1 = hashset_lookup(&set, key1);
    int result2 = hashset_lookup(&set, key2);

    assert_equal(1, result1, "Inserted key 1 should be found");
    assert_equal(1, result2, "Inserted key 2 should be found");

    print_test_result("Test 6: Testing Collision Handling", 1);
}

void test_stress(){
    HashSet set;
    hashset_init(&set);
    int n = 100000;
    uintptr_t base_address = 0x00000000000;
    for(int i = 0; i < n; i++){
        uintptr_t key = base_address + i;
        hashset_insert(&set, key);
    }

    for(int i = 0; i < n; i++){
        uintptr_t key = base_address + i;
        int result = hashset_lookup(&set, key);
        assert_equal(1, result, "Key should be found");
    }

    print_test_result("Test 7: Testing Stress", 1);
}


#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include "hashmap.h"

uintptr_t hash(uintptr_t *key, int size);

void print_test_result(char *test_name, int result);
void assert_equal(uintptr_t *expected, uintptr_t *actual, char *error_message);
void test_init();
void test_insert_and_lookup();
void test_multiple_inserts();
void test_update();
void test_iterator();
void test_collision_handling();
void test_stress(); 

int main() {
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

void print_test_result(char *test_name, int result) {
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
}

void assert_equal(uintptr_t *expected, uintptr_t *actual, char *error_message) {
    if(expected != actual) {
        printf("Assertion failed: %s\n", error_message);
        printf("Expected: %p, Actual: %p\n", (void*)expected, (void*)actual);
        exit(1);
    }
}

void test_init() {
    HashMap map;
    hashmap_init(&map);
    for(int i = 0; i < HASHMAP_SIZE; i++) {
        assert_equal(NULL, (uintptr_t *)map.buckets[i], "Buckets should be initialized");
    }
    print_test_result("Test 1: Testing Initialization", 1); 
}

void test_insert_and_lookup() {
    HashMap map;
    hashmap_init(&map);
    uintptr_t *key = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t *value = (uintptr_t *)0xfff000000001ULL;
    hashmap_insert(&map, key, value);
    
    uintptr_t *result = hashmap_lookup(&map, key);
    assert_equal(value, result, "Inserted value should be found");
    print_test_result("Test 2: Testing Insert and Lookup", 1); 
}

void test_multiple_inserts() {
    HashMap map;
    hashmap_init(&map);
    int n = 100;
    uintptr_t *base_address = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t *base_value = (uintptr_t *)0xfff000000001ULL;
    
    for(int i = 0; i < n; i++) {
        uintptr_t *key = (uintptr_t *)((char *)base_address + i * sizeof(uintptr_t));
        uintptr_t *value = (uintptr_t *)((char *)base_value + i * sizeof(uintptr_t));
        hashmap_insert(&map, key, value);
    }
    
    for(int i = 0; i < n; i++) {
        uintptr_t *key = (uintptr_t *)((char *)base_address + i * sizeof(uintptr_t));
        uintptr_t *value = (uintptr_t *)((char *)base_value + i * sizeof(uintptr_t));
        uintptr_t *result = hashmap_lookup(&map, key);
        assert_equal(value, result, "Inserted value should be found");
    }

    print_test_result("Test 3: Testing Multiple Inserts", 1);
}

void test_update() {
    HashMap map;
    hashmap_init(&map);
    uintptr_t *key = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t *value = (uintptr_t *)0xfff000000001ULL;
    uintptr_t *updated_value = (uintptr_t *)0xfff000000002ULL;
    hashmap_insert(&map, key, value);
    hashmap_insert(&map, key, updated_value);
    uintptr_t *result = hashmap_lookup(&map, key);
    assert_equal(updated_value, result, "Updated value should be found");
    print_test_result("Test 4: Testing Update", 1);
}

void test_iterator() {
    HashMap map;
    hashmap_init(&map);
    int n = 100;
    uintptr_t *base_address = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t *base_value = (uintptr_t *)0xfff000000001ULL;

    for(int i = 0; i < n; i++) {
        uintptr_t *key = (uintptr_t *)((char *)base_address + i * sizeof(uintptr_t));
        uintptr_t *value = (uintptr_t *)((char *)base_value + i * sizeof(uintptr_t));
        hashmap_insert(&map, key, value);
    }

    int count = 0;
    HashMapIterator *iter = hashmap_iterator_create(&map);
    uintptr_t *key = NULL, *value = NULL;
    while(hashmap_iterator_has_next(iter)) {
        hashmap_iterator_next(iter, key, value);
        count++;
    }

    hashmap_iterator_free(iter);
    print_test_result("Test 5: Testing Iterator", count == n);
}


void test_collision_handling() {
    uintptr_t *base = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t base_hash = hash(base, HASHMAP_SIZE);
    int found = 0;
    int i;
    
    for(i = 1; i < 10000; i++) {
        uintptr_t *key = (uintptr_t *)((char *)base + i * sizeof(uintptr_t));
        uintptr_t hash_value = hash(key, HASHMAP_SIZE);
        if(hash_value == base_hash) {
            found = 1;
            break;
        }
    }
    
    if(!found) {
        printf("No collision found\n");
        return;
    }

    HashMap map;
    hashmap_init(&map);
    uintptr_t *key1 = base;
    uintptr_t *value1 = (uintptr_t *)0xfff000000001ULL;
    uintptr_t *key2 = (uintptr_t *)((char *)base + i * sizeof(uintptr_t));
    uintptr_t *value2 = (uintptr_t *)0xfff000000002ULL;

    hashmap_insert(&map, key1, value1);
    hashmap_insert(&map, key2, value2);

    uintptr_t *result1 = hashmap_lookup(&map, key1);
    uintptr_t *result2 = hashmap_lookup(&map, key2);

    assert_equal(value1, result1, "Inserted value 1 should be found");
    assert_equal(value2, result2, "Inserted value 2 should be found");

    print_test_result("Test 6: Testing Collision Handling", 1);
}
void test_stress() {
    HashMap map;
    hashmap_init(&map);
    int n = 100000;
    uintptr_t *base_address = (uintptr_t *)0x7ff000000000ULL;
    uintptr_t *base_value = (uintptr_t *)0xfff000000001ULL;
    
    for(int i = 0; i < n; i++) {
        uintptr_t *key = (uintptr_t *)((char *)base_address + i * sizeof(uintptr_t));
        uintptr_t *value = (uintptr_t *)((char *)base_value + i * sizeof(uintptr_t));
        hashmap_insert(&map, key, value);
    }

    HashMapIterator *iter = hashmap_iterator_create(&map);
    int count = 0;
    uintptr_t *key = NULL, *value = NULL;
    
    while(hashmap_iterator_has_next(iter)) {
        hashmap_iterator_next(iter, key, value);  
        count++;
    }

    hashmap_iterator_free(iter);
    print_test_result("Test 7: Testing Stress", count == n);
}
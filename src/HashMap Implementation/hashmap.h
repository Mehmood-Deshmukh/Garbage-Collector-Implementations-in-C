#include <stdint.h>

typedef struct HashMapNode {
    uintptr_t key;
    uintptr_t value;
    struct HashMapNode *next;
} HashMapNode; 

typedef struct HashMap {
    HashMapNode **buckets;
    int size;
} HashMap;

typedef struct HashMapIterator {
    HashMap *map;
    int index;
    HashMapNode *node;
} HashMapIterator;

#define HASHMAP_SIZE 100

void hashmap_init(HashMap *map);
void hashmap_insert(HashMap *map, uintptr_t key, uintptr_t value);
void hashmap_delete(HashMap *map, uintptr_t key);
uintptr_t hashmap_lookup(HashMap *map, uintptr_t key);
void hashmap_free(HashMap *map);

HashMapIterator *hashmap_iterator_create(HashMap *map);
int hashmap_iterator_has_next(HashMapIterator *iter);
int hashmap_iterator_next(HashMapIterator *iter, uintptr_t *key, uintptr_t *value);
void hashmap_iterator_free(HashMapIterator *iter);
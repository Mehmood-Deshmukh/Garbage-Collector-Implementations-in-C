#include <stdint.h>

typedef struct HashSetNode {
    uintptr_t key;
    struct HashSetNode *next;
} HashSetNode;

typedef struct HashSet {
    HashSetNode **buckets;
    int size;
} HashSet;

typedef struct HashSetIterator {
    HashSet *set;
    int index;
    HashSetNode *node;
} HashSetIterator;

#define HASHSET_SIZE 1000

void hashset_init(HashSet *set);
void hashset_insert(HashSet *set, uintptr_t key);
int hashset_lookup(HashSet *set, uintptr_t key);
void hashset_delete(HashSet *set, uintptr_t key);
void hashset_free(HashSet *set);

HashSetIterator *hashset_iterator_create(HashSet *set);
int hashset_iterator_has_next(HashSetIterator *iter);
uintptr_t hashset_iterator_next(HashSetIterator *iter);
void hashset_iterator_free(HashSetIterator *iter);

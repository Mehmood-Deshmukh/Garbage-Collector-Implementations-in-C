#ifndef HASHSET_H
#define HASHSET_H

#include <stdint.h>


/*
This is the node structure for the hashmap.
Each node contains a key, value and a pointer to the next node in the chain.
*/

typedef struct HashSetNode {
    uintptr_t *key;
    struct HashSetNode *next;
} HashSetNode;

/*
This is the hashmap structure.
It contains an array of buckets which are pointers to the first node in the chain.
It also contains the size of the hashmap.
*/

typedef struct HashSet {
    HashSetNode **buckets;
    int size;
} HashSet;

/*
This is the iterator structure for the hashmap.
It is used to iterate over the elements in the hashmap.
It contains a pointer to the hashmap, the current index and the current node.
*/

typedef struct HashSetIterator {
    HashSet *set;
    int index;
    HashSetNode *node;
} HashSetIterator;

/*
This is the size of the hashmap.
by default, the size is set to 1000.
*/
#define HASHSET_SIZE 1000

/*
    function : hashset_init
    purpose : initialize the hashmap
    parameters : HashSet *set - pointer to the hashmap
    returns : void
*/
void hashset_init(HashSet *set);

/*
    function : hashset_insert
    purpose : insert a key into the hashmap
    parameters : HashSet *set - pointer to the hashmap
                 uintptr_t *key - key to insert
    returns : void
*/
void hashset_insert(HashSet *set, uintptr_t *key);

/*
    function : hashset_lookup
    purpose : lookup a key in the hashmap
    parameters : HashSet *set - pointer to the hashmap
                 uintptr_t *key - key to lookup
    returns : int - 1 if the key is found, 0 otherwise
*/
int hashset_lookup(HashSet *set, uintptr_t *key);

/*
    function : hashset_delete
    purpose : delete a key from the hashmap
    parameters : HashSet *set - pointer to the hashmap
                 uintptr_t *key - key to delete
    returns : void
*/
void hashset_delete(HashSet *set, uintptr_t *key);

/*
    function : hashset_free
    purpose : free the hashmap
    parameters : HashSet *set - pointer to the hashmap
    returns : void
*/
void hashset_free(HashSet *set);

/*
    function : hashset_iterator_create
    purpose : create an iterator for the hashmap
    parameters : HashSet *set - pointer to the hashmap
    returns : HashSetIterator * - pointer to the iterator
*/
HashSetIterator *hashset_iterator_create(HashSet *set);

/*
    function : hashset_iterator_has_next
    purpose : check if the iterator has more elements
    parameters : HashSetIterator *iter - pointer to the iterator
    returns : int - 1 if there are more elements, 0 otherwise
*/
int hashset_iterator_has_next(HashSetIterator *iter);

/*
    function : hashset_iterator_next
    purpose : get the next element from the iterator
    parameters : HashSetIterator *iter - pointer to the iterator
    returns : uintptr_t - the next element
*/
uintptr_t *hashset_iterator_next(HashSetIterator *iter);

/*
    function : hashset_iterator_free
    purpose : free the iterator
    parameters : HashSetIterator *iter - pointer to the iterator
    returns : void
*/
void hashset_iterator_free(HashSetIterator *iter);

#endif /* HASHSET_H */
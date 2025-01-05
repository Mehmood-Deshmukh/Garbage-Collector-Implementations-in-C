#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>

/*
    * HashMap Implementation

    * This is an implementation of a hashmap data structure in C.
    * It uses separate chaining to handle collisions.
    * The hashmap uses a hash function to map keys to indices in the hashmap.
    * The hash function used is MurmurHash3.
*/


/*
This is the node structure for the hashmap.
Each node contains a key, value and a pointer to the next node in the chain.
*/

typedef struct HashMapNode {
    uintptr_t *key;
    uintptr_t *value;
    struct HashMapNode *next;
} HashMapNode; 

/*
This is the hashmap structure.
It contains an array of buckets which are pointers to the first node in the chain.
It also contains the size of the hashmap.
*/

typedef struct HashMap {
    HashMapNode **buckets;
    int size;
} HashMap;

/*
This is the iterator structure for the hashmap.
It is used to iterate over the elements in the hashmap.
It contains a pointer to the hashmap, the current index and the current node.
*/

typedef struct HashMapIterator {
    HashMap *map;
    int index;
    HashMapNode *node;
} HashMapIterator;

/*
This is the size of the hashmap.
by default, the size is set to 100.
*/

#define HASHMAP_SIZE 100

/*
    function : hashmap_init
    purpose : initialize the hashmap
    parameters : HashMap *map - pointer to the hashmap
    returns : void
*/
void hashmap_init(HashMap *map);

/*
    function : hashmap_insert
    purpose : insert a key-value pair into the hashmap
    parameters : HashMap *map - pointer to the hashmap
                 uintptr_t *key - key to insert
                 uintptr_t *value - value to insert
    returns : void
*/
void hashmap_insert(HashMap *map, uintptr_t *key, uintptr_t *value);

/*
    function : hashmap_delete
    purpose : delete a key from the hashmap
    parameters : HashMap *map - pointer to the hashmap
                 uintptr_t *key - key to delete
    returns : void
*/
void hashmap_delete(HashMap *map, uintptr_t *key);

/*
    function : hashmap_lookup
    purpose : lookup a key in the hashmap
    parameters : HashMap *map - pointer to the hashmap
                 uintptr_t *key - key to lookup
    returns : uintptr_t *- value associated with the key
*/
uintptr_t *hashmap_lookup(HashMap *map, uintptr_t *key);

/*
    function : hashmap_free
    purpose : free the hashmap
    parameters : HashMap *map - pointer to the hashmap
    returns : void
*/
void hashmap_free(HashMap *map);

/*
    function : hashmap_iterator_create
    purpose : create an iterator for the hashmap
    parameters : HashMap *map - pointer to the hashmap
    returns : HashMapIterator * - pointer to the iterator
*/
HashMapIterator *hashmap_iterator_create(HashMap *map);

/*
    function : hashmap_iterator_has_next
    purpose : check if the iterator has more elements
    parameters : HashMapIterator *iter - pointer to the iterator
    returns : int - 1 if there are more elements, 0 otherwise
*/
int hashmap_iterator_has_next(HashMapIterator *iter);

/*
    function : hashmap_iterator_next
    purpose : get the next key-value pair from the iterator
    parameters : HashMapIterator *iter - pointer to the iterator
                 uintptr_t *key - pointer to store the key
                 uintptr_t *value - pointer to store the value
    returns : int - 1 if successful, 0 otherwise
*/
int hashmap_iterator_next(HashMapIterator *iter, uintptr_t *key, uintptr_t *value);

/*
    function : hashmap_iterator_free
    purpose : free the iterator
    parameters : HashMapIterator *iter - pointer to the iterator
    returns : void
*/
void hashmap_iterator_free(HashMapIterator *iter);

#endif
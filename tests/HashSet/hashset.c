#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "hashset.h"
#include "../../src/Hash-Functions/hash_functions.h"

void hashset_init(HashSet *set){
    set->buckets = malloc(HASHSET_SIZE * sizeof(HashSetNode *));
    set->size = HASHSET_SIZE;

    for(int i = 0; i < HASHSET_SIZE; i++){
        set->buckets[i] = NULL;
    }
}


void hashset_insert(HashSet *set, uintptr_t *key){
    if(hashset_lookup(set, key)) return;

    uintptr_t index = hash(key, set->size);

    HashSetNode *node = malloc(sizeof(HashSetNode));
    node->key = key;
    node->next = set->buckets[index];
    set->buckets[index] = node;
}

int hashset_lookup(HashSet *set, uintptr_t *key){
    uintptr_t index = hash(key, set->size);

    HashSetNode *node = set->buckets[index];
    while(node){
        if(node->key == key){
            return 1;
        }
        node = node->next;
    }

    return 0;
}

void hashset_delete(HashSet *set, uintptr_t *key){
    uintptr_t index = hash(key, set->size);

    HashSetNode *node = set->buckets[index];
    HashSetNode *prev = NULL;

    while(node){
        if(node->key == key){
            if(prev){
                prev->next = node->next;
            } else {
                set->buckets[index] = node->next;
            }
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void hashset_free(HashSet *set){
    for(int i = 0; i < set->size; i++){
        HashSetNode *node = set->buckets[i];
        while(node){
            HashSetNode *temp = node;
            node = node->next;
            free(temp);
        }
    }

    free(set->buckets);
    set->buckets = NULL;
    set->size = 0;
}

HashSetIterator *hashset_iterator_create(HashSet *set){
    HashSetIterator *iter = malloc(sizeof(HashSetIterator));
    iter->set = set;
    iter->index = 0;
    iter->node = NULL;

    while(iter->index < set->size && !set->buckets[iter->index]){
        iter->index++;
    }

    if(iter->index < set->size){
        iter->node = set->buckets[iter->index];
    }

    return iter;
}

int hashset_iterator_has_next(HashSetIterator *iter){
    return iter->node != NULL;
}

uintptr_t *hashset_iterator_next(HashSetIterator *iter){
    if(!hashset_iterator_has_next(iter)) return 0;

    uintptr_t *key = iter->node->key;

    iter->node = iter->node->next;
    if(!iter->node){
        iter->index++;
        while(iter->index < iter->set->size && !iter->set->buckets[iter->index]){
            iter->index++;
        }

        if(iter->index < iter->set->size){
            iter->node = iter->set->buckets[iter->index];
        }
    }

    return key;
}

void hashset_iterator_free(HashSetIterator *iter){
    free(iter);
}
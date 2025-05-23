#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "hashmap.h"
#include "../../src/Hash-Functions/hash_functions.h"

void hashmap_init(HashMap *map){
    map->buckets = malloc(HASHMAP_SIZE * sizeof(HashMapNode *));
    map->size = HASHMAP_SIZE;

    for(int i = 0; i < HASHMAP_SIZE; i++){
        map->buckets[i] = NULL;
    }
}

void hashmap_insert(HashMap *map, uintptr_t *key, uintptr_t *value){
    if(hashmap_lookup(map, key)){
        hashmap_delete(map, key);
    }
    
    uintptr_t index = hash(key, map->size);

    HashMapNode *node = malloc(sizeof(HashMapNode));
    node->key = key;
    node->value = value;
    node->next = map->buckets[index];
    map->buckets[index] = node;
}

uintptr_t *hashmap_lookup(HashMap *map, uintptr_t *key){
    uintptr_t index = hash(key, map->size);

    HashMapNode *node = map->buckets[index];
    while(node){
        if(node->key == key){
            return node->value;
        }
        node = node->next;
    }

    return 0;
}

void hashmap_delete(HashMap *map, uintptr_t *key){
    uintptr_t index = hash(key, map->size);

    HashMapNode *node = map->buckets[index];
    HashMapNode *prev = NULL;

    while(node){
        if(node->key == key){
            if(prev){
                prev->next = node->next;
            } else {
                map->buckets[index] = node->next;
            }
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void hashmap_free(HashMap *map){
    for(int i = 0; i < map->size; i++){
        HashMapNode *node = map->buckets[i];
        while(node){
            HashMapNode *temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->size = 0;
}


HashMapIterator *hashmap_iterator_create(HashMap *map){
    HashMapIterator *iter = malloc(sizeof(HashMapIterator));
    iter->map = map;
    iter->index = 0;
    iter->node = NULL;

    while(iter->index < map->size && !map->buckets[iter->index]){
        iter->index++;
    }

    if(iter->index < map->size){
        iter->node = map->buckets[iter->index];
    }

    return iter;
}

int hashmap_iterator_has_next(HashMapIterator *iter){
    return iter->node != NULL;
}

int hashmap_iterator_next(HashMapIterator *iter, uintptr_t *key, uintptr_t *value){
    if(!hashmap_iterator_has_next(iter)) return 0;

    key = iter->node->key;
    value = iter->node->value;

    iter->node = iter->node->next;
    if(!iter->node){
        iter->index++;
        while(iter->index < iter->map->size && !iter->map->buckets[iter->index]){
            iter->index++;
        }

        if(iter->index < iter->map->size){
            iter->node = iter->map->buckets[iter->index];
        }
    }

    return 1;
}

void hashmap_iterator_free(HashMapIterator *iter){
    free(iter);
}
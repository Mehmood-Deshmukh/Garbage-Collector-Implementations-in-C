#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "hashmap.h"

uintptr_t hash(uintptr_t key, int size);
uintptr_t murmur_hash3(uintptr_t key);
uint32_t generate_seed(void);
uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed);
uint32_t getblock32(const uint32_t *p, int i);
uint32_t ROTL32(uint32_t x, int y);



void hashmap_init(HashMap *map){
    map->buckets = malloc(HASHMAP_SIZE * sizeof(HashMapNode *));
    map->size = HASHMAP_SIZE;

    for(int i = 0; i < HASHMAP_SIZE; i++){
        map->buckets[i] = NULL;
    }
}

uintptr_t hash(uintptr_t key, int size) {
    return murmur_hash3(key) % size;
}

uintptr_t murmur_hash3(uintptr_t key) {
    return (uintptr_t)murmurhash3_x86_32(&key, sizeof(uintptr_t), generate_seed());
}

uint32_t generate_seed(void) {
    time_t current_time;
    time(&current_time);
    return (uint32_t)current_time;
}

uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed) {
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 4;
    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    const uint32_t *blocks = (const uint32_t *)(data);
    for (int i = 0; i < nblocks; i++) {
        uint32_t k1 = getblock32(blocks, i);
        
        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;
        
        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }
    
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3:
            k1 ^= tail[2] << 16;
            
        case 2:
            k1 ^= tail[1] << 8;
            
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

uint32_t getblock32(const uint32_t *p, int i) {
    return p[i];
}

uint32_t ROTL32(uint32_t x, int y) {
    return (x << y) | (x >> (32 - y));
}

void hashmap_insert(HashMap *map, uintptr_t key, uintptr_t value){
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

uintptr_t hashmap_lookup(HashMap *map, uintptr_t key){
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

void hashmap_delete(HashMap *map, uintptr_t key){
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

    *key = iter->node->key;
    *value = iter->node->value;

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
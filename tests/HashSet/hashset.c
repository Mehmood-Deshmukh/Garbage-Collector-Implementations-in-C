#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "hashset.h"

uintptr_t hash(uintptr_t key, int size);
uintptr_t murmur_hash3(uintptr_t key);
uint32_t generate_seed(void);
uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed);
uint32_t getblock32(const uint32_t *p, int i);
uint32_t ROTL32(uint32_t x, int y);

void hashset_init(HashSet *set){
    set->buckets = malloc(HASHSET_SIZE * sizeof(HashSetNode *));
    set->size = HASHSET_SIZE;

    for(int i = 0; i < HASHSET_SIZE; i++){
        set->buckets[i] = NULL;
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

void hashset_insert(HashSet *set, uintptr_t key){
    if(hashset_lookup(set, key)) return;

    uintptr_t index = hash(key, set->size);

    HashSetNode *node = malloc(sizeof(HashSetNode));
    node->key = key;
    node->next = set->buckets[index];
    set->buckets[index] = node;
}

int hashset_lookup(HashSet *set, uintptr_t key){
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

void hashset_delete(HashSet *set, uintptr_t key){
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

uintptr_t hashset_iterator_next(HashSetIterator *iter){
    if(!hashset_iterator_has_next(iter)) return 0;

    uintptr_t key = iter->node->key;

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
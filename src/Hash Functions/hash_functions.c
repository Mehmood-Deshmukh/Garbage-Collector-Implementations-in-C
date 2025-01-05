#include "hash_functions.h"
#include <time.h>
#include <stdint.h>

uintptr_t murmur_hash3(uintptr_t key);
uint32_t generate_seed(void);
uint32_t murmurhash3_x86_32(const void *key, size_t len, uint32_t seed);
uint32_t getblock32(const uint32_t *p, int i);
uint32_t ROTL32(uint32_t x, int y);

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
/* ### HashSet in C

- A hashset is a data structure that stores a set of unique elements.
- We will use it to store the set of valid addresses in the heap
- We can implement a HashSet using HashMap but I will like to keep a seperate Implementation.

### HashSet Implementation
1. **The HashSet Structure**
    - The hashset structure will store the array of linked lists.
    ```c
    typedef struct HashSet {
        HashSetNode **buckets;
        int size;
    } HashSet;
    ```
    - The `buckets` field is an array of linked lists.
    - The `size` field is the size of the array.
    - The HashSetNode structure will store the key.
    ```c   
    typedef struct HashSetNode {
        uintptr_t key;
        struct HashSetNode *next;
    } HashSetNode;
    ```
2. **The HashSet Functions**
    1. HashSet Initialization
        - The `hashset_init()` function will initialize the hashset.
        ```c
        void hashset_init(HashSet *set, int SIZE){
            set->buckets = malloc(SIZE * sizeof(HashSetNode *));
            set->size = SIZE;

            for(int i = 0; i < SIZE; i++){
                set->buckets[i] = NULL;
            }
        }
        ```
    2. HashSet Insertion
        - The `hashset_insert()` function will insert a key into the hashset.
        - We will use the same hash function as the hashmap.
        ```c
        void hashset_insert(HashSet *set, uintptr_t key){
            uintptr_t index = hash(key, set->size);

            HashSetNode *node = malloc(sizeof(HashSetNode));
            node->key = key;
            node->next = set->buckets[index];
            set->buckets[index] = node;
        }
        ```
    3. HashSet Lookup
        - The `hashset_lookup()` function will lookup if a key is present in the hashset.
        ```c
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
        ```
    4. HashSet Deletion
        - The `hashset_delete()` function will delete a key from the hashset.
        ```c
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
        ```
    5. HashSet Free
        - The `hashset_free()` function will free the hashset.
        ```c
        void hashset_free(HashSet *set){
            for(int i = 0; i < set->size; i++){
                HashSetNode *node = set->buckets[i
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
        ```
    6. HashSet Iterator
        - The `hashset_iterator_create()` function will create an iterator for the hashset.
        ```c
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
        ``` */


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

#define SIZE 100

void hashset_init(HashSet *set);
void hashset_insert(HashSet *set, uintptr_t key);
int hashset_lookup(HashSet *set, uintptr_t key);
void hashset_delete(HashSet *set, uintptr_t key);
void hashset_free(HashSet *set);

HashSetIterator *hashset_iterator_create(HashSet *set);
int hashset_iterator_has_next(HashSetIterator *iter);
uintptr_t hashset_iterator_next(HashSetIterator *iter);
void hashset_iterator_free(HashSetIterator *iter);

#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"

void print_hashset(HashSet *set);
void print_hashmap(HashMap *map);
void print_linked_list();

void gc_init() {
    gc.stack_top = __builtin_frame_address(1);
    gc.address = malloc(sizeof(HashSet));
    gc.metadata = malloc(sizeof(HashMap));
    gc.list_head = gc.list_tail = NULL;
    gc.total_allocated = 0;


    int *a = (int *)malloc(sizeof(int));
    gc.stack_bottom = &a;
    free(a);

    if(!gc.address || !gc.metadata){
        printf("Unable to allocate memory for gc initialization\n");
        exit(1);
    }

    hashset_init(gc.address);
    hashmap_init(gc.metadata);
}

HashMap *get_roots(){
    jmp_buf jb;
    setjmp(jb);

    HashMap *roots = malloc(sizeof(HashMap));
    if(!roots){
        printf("Unable to allocate memory for roots\n");
        exit(1);
    }
    hashmap_init(roots);

    uint8_t *stack_bottom = (uint8_t *)gc.stack_bottom + sizeof(uintptr_t);
    uint8_t *stack_top = (uint8_t *)gc.stack_top;


    while(stack_bottom < stack_top){
        uintptr_t *address = (uintptr_t *)*(uintptr_t *)stack_bottom;
        if(hashset_lookup(gc.address, address)){
            hashmap_insert(roots, (uintptr_t *)stack_bottom, address);
        }
        stack_bottom += sizeof(uintptr_t);
    }

    return roots;
}

HashSet *get_children(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return NULL;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(!metadata) return NULL;

    HashSet *children = malloc(sizeof(HashSet));
    if(!children){
        printf("Unable to allocate memory for children\n");
        exit(1);
    }
    hashset_init(children);

    uint8_t *start = (uint8_t *)address;
    uint8_t *end = (uint8_t *)((uint8_t *)address + metadata->size);

    while(start < end){
        uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;

        if(((uintptr_t) address % sizeof(uintptr_t)) == 0){
            if(hashset_lookup(gc.address, address)){
                hashset_insert(children, address);
            }
        }

        start += sizeof(uintptr_t);
    }
    return children;
}

void gc_mark_helper(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
    if(!metadata || metadata->marked) return;

    metadata->marked = 1;

    HashSet *children = get_children(address);
    if(!children) return;
    HashSetIterator *iterator = hashset_iterator_create(children);
    if(!iterator){
        hashset_free(children);
        free(children);
        return;
    }

    while(hashset_iterator_has_next(iterator)){
        gc_mark_helper(hashset_iterator_next(iterator));
    }

    hashset_free(children);
    free(children);
    hashset_iterator_free(iterator);
}

void gc_mark(HashMap *roots){
    if(!roots) return;

    HashMapIterator *iterator = hashmap_iterator_create(roots);
    if(!iterator) return;

    uintptr_t *key;
    uintptr_t *value;

    while(hashmap_iterator_has_next(iterator)){
        hashmap_iterator_next(iterator, &key, &value);
        gc_mark_helper(value);
    }

    hashmap_iterator_free(iterator);
}

void gc_sweep(){
    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    if(!iterator) return;

    while(hashset_iterator_has_next(iterator)){
        uintptr_t *address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        if(!metadata) continue; 

        if(metadata->marked == 0){
            gc_free(address);
        } else {
            metadata->marked = 0;
        }
    }

    hashset_iterator_free(iterator);
}

void compute_locations(){
    MetaData *live = gc.list_head;
    MetaData *free = gc.list_head;

    while(live){
        if(live->marked){
            live->forwarding_address = free->address;
            free = free->next;
        }

        live = live->next;
    }
}

void update_references(HashMap *roots){
    HashMapIterator *iterator = hashmap_iterator_create(roots);
    uintptr_t *key;
    uintptr_t *value;

    while(hashmap_iterator_has_next(iterator)){
        hashmap_iterator_next(iterator, &key, &value);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, value);
        if(metadata){
            uintptr_t *new_address = (uintptr_t *)metadata->forwarding_address;
            if(new_address){
                *key = (uintptr_t)new_address;
            }
        }
    }

    MetaData *temp = gc.list_head;

    while(temp){
        uint8_t *start = temp->address;
        uint8_t *end = temp->address + temp->size;

        while(start < end){
            uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;
            MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
            if(metadata){
                uintptr_t *new_address = (uintptr_t *)metadata->forwarding_address;
                if(new_address){
                    *(uintptr_t *)start = (uintptr_t)new_address;
                }
            }
            
            start += sizeof(uintptr_t);
        }

        temp = temp->next;
    }


    hashmap_iterator_free(iterator);
}

void relocate(){
    MetaData *temp = gc.list_head;
    int total_garbage = 0;

    while(temp){
        if(temp->marked){
            uintptr_t *destination = (uintptr_t *)temp->forwarding_address;
            MetaData *destination_metadata = (MetaData *)hashmap_lookup(gc.metadata, destination);
            uint8_t *source = temp->address;
        
            memcpy(destination, source, temp->size);
            destination_metadata->size = temp->size;
            destination_metadata->marked = 1;
        }else{
            total_garbage ++;
        }

        temp = temp->next;
    }
    temp = gc.list_head;
    int total_live_objects = gc.total_allocated - total_garbage;

    for(int i = 0; i < total_live_objects; i++){
        temp = temp->next;
    }

    while(temp){
        temp->marked = 0;
        temp = temp->next;
    }
}

void gc_compact(HashMap *roots){
    compute_locations();
    update_references(roots);
    relocate();
}


void gc_run(){
    HashMap *roots = get_roots();
    if(!roots) return;
    print_linked_list();
    gc_mark(roots);
    gc_dump("After Marking");
    gc_compact(roots); 
    gc_sweep();

    hashmap_free(roots);
    free(roots);
}

void gc_dump(char *message){
    printf("%s\n\n", message);
    printf("{\n");

    HashSetIterator *iterator = hashset_iterator_create(gc.address);
    if(!iterator) return;

    int count = 0;
    while(hashset_iterator_has_next(iterator)){
        count++;
        uintptr_t *address = hashset_iterator_next(iterator);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
        if(!metadata) continue;
        printf("\t%p : {marked: %d, size: %zu},\n", address, metadata->marked, metadata->size);
    }
    printf("\n\nTotal Allocated: %d\n", count);
    printf("}\n");

    hashset_iterator_free(iterator);
}

void *gc_malloc(size_t size){
    if(size == 0) return NULL;

    void *address = (void *)calloc(1, size);
    if(!address){
        printf("Unable to allocate memory for size %zu\n", size);
        exit(1);
    }

    MetaData *metadata = (MetaData *)malloc(sizeof(MetaData));
    if(!metadata){
        printf("Unable to allocate memory for metadata\n");
        exit(1);
    }

    metadata->marked = 0;
    metadata->size = size;
    metadata->address = (uint8_t *)address;
    metadata->forwarding_address = NULL;
    metadata->next = NULL;

    hashset_insert(gc.address, address);
    hashmap_insert(gc.metadata, address, (uintptr_t *)metadata);

    if(!gc.list_head){
        gc.list_head = metadata;
        gc.list_tail = metadata;
    } else {
        gc.list_tail->next = metadata;
        gc.list_tail = metadata;
    }

    gc.total_allocated++;

    return address;
}

void gc_free(uintptr_t *address){
    if(!address || !hashset_lookup(gc.address, address)) return;

    MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);

    MetaData *temp = gc.list_head;
    MetaData *prev = NULL;
    while(temp){
        if((uintptr_t *)temp->address == address){
            if(prev){
                prev->next = temp->next;
            } else {
                gc.list_head = temp->next;
            }

            if(temp == gc.list_tail){
                gc.list_tail = prev;
            }


            break;
        }
        prev = temp;
        temp = temp->next;
    }


    hashset_delete(gc.address, address);
    hashmap_delete(gc.metadata, address);

    if(metadata) free(metadata);
    gc.total_allocated--;
    free(address);
}


void print_hashset(HashSet *set){
    printf("====================\n");
    HashSetIterator *iterator = hashset_iterator_create(set);
    while(hashset_iterator_has_next(iterator)){
        uintptr_t *address = hashset_iterator_next(iterator);
        printf("%p\n", address);
    }
    hashset_iterator_free(iterator);
    printf("====================\n");
}

void print_hashmap(HashMap *map){
    printf("====================\n");
    HashMapIterator *iterator = hashmap_iterator_create(map);
    uintptr_t *key;
    uintptr_t *value;
    while(hashmap_iterator_has_next(iterator)){
        hashmap_iterator_next(iterator, &key, &value);
        printf("%p : %p\n", key, value);
    }
    hashmap_iterator_free(iterator);
    printf("====================\n");
}

void print_linked_list(){
    printf("====================\n");
    MetaData *temp = gc.list_head;
    while(temp){
        printf("%p -> ", temp->address);
        temp = temp->next;
    }
    printf("NULL\n");
    printf("====================\n");
}


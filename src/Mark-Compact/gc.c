#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h> /* for setjmp */
#include <string.h>

/*
 * Pragma is a compiler directive that provides additional information to the compiler.
 * diagnostic - This pragma is used to control the compiler's warning and error messages.
 * the first pragma is used to push the compiler's current warning state onto a stack. 
 * This allows us to temporarily change the warning state and then restore it later.
 * -Wframe-address - This warning is generated to let the user know that
 * "You’re using __builtin_frame_address or __builtin_return_address. These are not always 
 * reliable, they’re non-standard, might not work in optimized code, etc"
 * 
 * We are suppressing this warning.
 */


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"


/* used for debugging */
void print_hashset(HashSet *set);
void print_hashmap(HashMap *map);
void print_linked_list();

/* This is the actual instance of the garbage collector. */
GC gc;

/*
 * About this function:
 * 
 * This function initializes the garbage collector.
 * It does the following:
 * 1. Sets the stack_top to the main' frame address.
 *   - This is done using __builtin_frame_address(1) which gives the frame address
 *     of the caller function (in this case, main).
 * 2. Allocates memory for the address set and metadata map.
 *   - The address set is a HashSet that will store all the allocated addresses.
 *   - The metadata map is a HashMap that will store the metadata of the objects.
 * 3. getting the stack_bottom address.
 *   - This is done by allocating a temporary integer pointer, and then setting
 *     stack_bottom to the address of that pointer. credits - Aditya Deshmukh
 * 4. Initializes the address set and metadata map.
 * 
 * 
 * This must be the first function to be called before using the garbage collector. 
 * 
 * Additions for Mark-Compact:
 * 
 * We will initialize the head and tail to NULL and also the total_allocated to 0.
 */


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

/* 
 * About this function:
 * 
 * This function helps to get the roots, the starting points of the garbage collection.
 * These are the objects that are guaranteed to be reachable from the program.
 *  What are these objects? - those reachable from the objects on the stack.
 * 
 * So our task is simple - just scan the stack and get the roots.
 * 
 * However, a simple addition : sometimes the compiler can choose to store
 * the objects in registers instead of the stack.
 * 
 * How can we access this? 
 * we use setjmp function from the setjmp.h library.
 * setjmp is a function that saves the contents of registers 
 * in a jmp_buf structure and now we can access them
 * 
 * How it works:
 * 1. We create a jmp_buf variable to store the state of the registers and call the setjmp function.
 * 2. We allocate memory for the roots HashSet.
 * 3. We get the stack_bottom and stack_top addresses from the gc instance.
 * 4. We iterate over the stack from stack_bottom to stack_top.
 *    - for each pointer like value in the stack, we check if it is a valid address
 *      in the garbage collector's address set.
 *   - if it is, we insert it into the roots HashSet.
 * 5. Finally, we return the roots HashSet.
 * 
 * Additions for Mark-Compact:
 * 
 * In Mark and Compact, we use a HashMap instead of a HashSet to store the roots.
 * Why is that? 
 * Because when we use a mark and compact algorithm, the objects are moved in a heap
 * so their addresses change.
 * suppose a pointer in the stack points to an object in the heap.
 * if we use a hashmap to store the address of the pointer along with the address of the object to
 * which it points, we can easily update the pointer's address after the compaction.
 * on the other hand, if we use a HashSet, we will not be able to update the pointer's address
 * 
 * 
 */

HashMap *get_roots(){
    jmp_buf jb;
    setjmp(jb);

    HashMap *roots = malloc(sizeof(HashMap));
    if(!roots){
        printf("Unable to allocate memory for roots\n");
        exit(1);
    }
    hashmap_init(roots);

    uintptr_t *stack_bottom = (uintptr_t *)gc.stack_bottom + 1;
    uintptr_t *stack_top = (uintptr_t *)gc.stack_top;


    while(stack_bottom < stack_top){
        uintptr_t *address = (uintptr_t *)*stack_bottom;
        if(hashset_lookup(gc.address, address)){
            hashmap_insert(roots, stack_bottom, address);
        }
        stack_bottom++;
    }

    return roots;
}



/* 
 * About this function:
 *
 * This function returns the children of the object at the given address.
 * What are children?
 * - As we are implementing a "conservative" garbage collector, we consider
 * any "pointer-like" value in the memory as a pointer to another object.
 * So, the children of an object are the pointer-like values that point to
 * valid addresses in the garbage collector's address set.
 * 
 * 
 * This is our funny little "Duck Test" - if it looks like a duck, swims like a duck,
 * and quacks like a duck, then it is a duck.
 * 
 * In our case, if it looks like a pointer, and points to a valid address, 
 * then it is a pointer. reference: (http://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/)
 * 
 * - This is a conservative approach because it may include values that are not
 * actually pointers, but we treat them as such.
 * 
 * for example:
 *   int *ptr = malloc(sizeof(int));
 *   int a = (int) ptr;  This is a pointer-like value, but it is not a pointer.
 *   ptr = null
 * 
 * if we run our garbage collector after this code, it will consider `a` as a pointer
 * 
 * 
 * How it works:
 * 
 * 1. check if the address is valid and exists in the garbage collector's address set.
 * 2. get the metadata for the address from the hashmap.
 * 3. create a new HashSet to store the children.
 * 4. iterate over the memory block of the object at the given address.
 *    - Now, initially i thought that i need to keep a window of size of a pointer
 *      and move that window by one byte at a time. 
 *    - This seemed reasonable right? but C makes our life easier. pointers are always
 *      aligned to the size of a pointer, so we can just iterate over the memory
 *      block by the size of a pointer.
 *    - for each pointer-like value in the memory block, check if it is a valid address
 *      in the garbage collector's address set.
 *    - if it is, insert it into the children HashSet, else ignore it.
 *    - increment the start pointer by the size of a pointer.
 * 5. return the children HashSet.
 * 
 * Now, let's see the most confusing part, the scan:
 * - The starting point will be the address of the object.
 * - The end point will be the address of the object + size of the object.
 * - We will iterate over the memory block from start to end, checking each pointer-like value
 *   to see if it is a valid address in the garbage collector's address set.
 * - Now, if you read the code you'll find a lot of typecasting and pointer arithmetic,
 *   but don't worry, i'll try to explain it in a simple way.
 * 
 * 1. start = address;
 * 2. end = (uintptr_t *)((uint8_t *)address + metadata->size);
 *   - Here, we are calculating the end address by adding the size of the object
 *    to the address of the object. Why did we cast address to (uint8_t *)?
 *   - The answer lies in "pointer arithmetic". In c pointers are incremented by
 *     the size of the type they point to.
 *     for example, if you have a pointer to an int, and you increment it by 1,
 *     it will actually increment by sizeof(int) bytes.
 *   - so if we simply perform address + metadata->size, it will not give us the
 *    correct end address, because it will increment by the size of a pointer * metadata->size.
 *   - So, we cast the address to (uint8_t *) which is a byte pointer, and then increment it
 *     by metadata->size bytes to get the correct increment of metadata->size * 1 bytes.
 * 
 * 3. While start < end:
 *   - We will iterate over the memory block from start to end, checking each pointer-like  
 *     value to see if it is a valid address in the garbage collector's address set.
 *   - uintptr_t *address = (uintptr_t *)*(uintptr_t *)start;
 *   - Here, we are dereferencing the start pointer to get the value at that address,
 *     and then casting it to a uintptr_t pointer. This is because we are treating the
 *     value at that address as a pointer-like value.
 *   - if(((uintptr_t) address % sizeof(uintptr_t)) == 0){
 *   - This checks if the address is aligned to the size of a pointer. If it is,
 *     we consider it as a valid pointer-like value.
 *   - if(hashset_lookup(gc.address, address)){
 *   - This checks if the address is a valid address in the garbage collector's address set.
 *   - if it is, we insert it into the children HashSet.
 *
 */

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

/* 
 * About this function:
 * This function is a helper function for the gc_mark function.
 * It marks the object at the given address and recursively marks all its children.
 * 
 * How it works:
 *     1. check whether the address is NULL or not in the garbage collector's address set.
 *        if it is, return.
 *     2. get the metadata for the address from the hashmap.
 *        if metadata is NULL or already marked, return.
 *     3. set the marked field of the metadata to 1, indicating that the object is reachable.
 *     4. recursively mark the childrens of the object
 * 
 */  


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
/* 
 * About this function : 
 * 
 * Firstly, marking means setting the marked field of the metadata to 1. This means that
 * the object is reachable and should not be collected by the garbage collector.
 * 
 * This function marks takes the set of roots as input and for each root:
 *     1. it marks the root
 *     2. it marks all the children of the root recursively.
 * 
 * This is done using the gc_mark_helper function
 * 
 * At the end we will end up with all the reachable objects marked. 
 * 
 * Additions for Mark-Compact:
 * In Mark-Compact, we use a HashMap instead of a HashSet to store the roots.
 * but if we use only the values of hashmap, then there is no need to change the
 * function and we can use the same function as in Mark and Sweep.
 */

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

/* 
 * About this function:
 *
 * This is probably the simplest yet most important function in the garbage collector.
 * It is responsible for sweeping the memory and freeing the unmarked objects.
 * How it works:
 * 
 * 1. iterate through all the addresses in the garbage collector's address set.
 * 2  if the object is not marked, it means that it is unreachable and can be freed.
 * 3. if the object is marked, we reset the marked field to 0, for the next garbage collection cycle.
 */

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

/* 
 * About this function:
 * 
 * This is the simplest of the three sub-functions of the gc_compact function.
 * It computes the forwarding addresses for the live objects.
 * 
 * 1. It iterates through the linked list of live objects 
 * 2. If the object is marked, it sets the forwarding address of the object to the
 * next free address in the linked list
 * 
 * How does it work?
 * - we maintain two pointers, one for the live objects and one for the free objects.
 * - initially both pointers point to the head of the linked list.
 * - we iterate through the live objects and for each live object that is marked,
 *  we set its forwarding address to the address of the next free object.
 * - we then move the free pointer to the next free object.
 * * This way, we compute the forwarding addresses for all the live objects in the linked list.
 * 
 * Assume there are three live objects in the linked list, with middle one not marked.
 * 1. live and free point to the first object.
 * 2. because the first object is marked, we set its forwarding address to itself
 * 3. both live and free pointers move to the second object.
 * 4. the second object is not marked, so we do not set its forwarding address. 
 * 5. only live is moved to the third object, free is still pointing to the second object.
 * 6. the third object is marked, so we set its forwarding address to the address pointed
 * by free pointer, which is the second object.
 * 
 * This way, we have computed the forwarding addresses for all the live objects.
 * 
 * 
 */

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

/* 
 * About this function:
 * This function updates the references to the live objects after compaction.
 * How it works:
 * 1. It iterates through the roots HashMap and for each root
 *    - It updates the address the root is pointing to, to the forwarding address of the object.
 * 
 * 2. It iterates through the linked list of live objects and for each object
 *    - It scans the entire object looking for pointer-like values, which point to valid addresses. 
 *    - it updates those pointer-like values to point to the forwarding address of the object.
 * 
 * This way at the end of this function, all the references to the live objects have been updated
 * to their forwarding addresses.
 */

void update_references(HashMap *roots){
    HashMapIterator *iterator = hashmap_iterator_create(roots);
    uintptr_t *key;
    uintptr_t *value;

    while(hashmap_iterator_has_next(iterator)){
        hashmap_iterator_next(iterator, &key, &value);
        MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, value);
        if(metadata){
            uintptr_t *new_address = metadata->forwarding_address;
            if(new_address){
                *key = (uintptr_t)new_address;
            }
        }
    }

    MetaData *temp = gc.list_head;

    while(temp){
        uintptr_t *start = temp->address;
        uintptr_t *end = (uintptr_t *)((uint8_t *)temp->address + temp->size);

        while(start < end){
            uintptr_t *address = (uintptr_t *)*start;
            MetaData *metadata = (MetaData *)hashmap_lookup(gc.metadata, address);
            if(metadata){
                uintptr_t *new_address = metadata->forwarding_address;
                if(new_address){
                    *start = (uintptr_t)new_address;
                }
            }
            
            start ++;
        }

        temp = temp->next;
    }


    hashmap_iterator_free(iterator);
}


/* 
 * About this function:
 * 
 * This function is responsible for relocating the live objects to their new addresses.
 * It does the following:
 * 1. Iterates through the linked list of live objects and for each object
 *    - If the object is marked, it copies the object to its forwarding address.
 *    - If the object is not marked, it counts it as garbage.
 * 2. After copying all the live objects, i worked around to mark the garbage blocks as 0
 *  - I did this by maintaining a count of garbage objects 
 *  - After compacting, all the live objects are at the beginning of the list
 *  - after traversing the first `total_live_objects` objects, we reset the marked field of all the objects
 *   to 0, so that they can be marked again in the next garbage collection cycle
 * 
 * - total_live_objects = total_allocated - total_garbage;
 * 
 * This way, we have relocated all the live objects to their new addresses and freed the garbage.
 */

void relocate(){
    MetaData *temp = gc.list_head;
    int total_garbage = 0;

    while(temp){
        if(temp->marked){
            uintptr_t *destination = temp->forwarding_address;
            MetaData *destination_metadata = (MetaData *)hashmap_lookup(gc.metadata, destination);
            uintptr_t *source = temp->address;
        
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

/* 
 * About this function:
 * 
 * This function is responsible for compacting the memory.
 * It does the following:
 * 1. Computes the forwarding addresses for the live objects by calling compute_locations function.
 * 2. Updates the references to the live objects by calling update_references function.
 * 3. Relocates the live objects to their new addresses by calling relocate function.
 */

void gc_compact(HashMap *roots){
    compute_locations();
    update_references(roots);
    relocate();
}

/* 
 * About this function:
 * This is the main function that runs the garbage collector, which is accessible to the user.
 * 
 * It does the following:
 *    1. Gets the roots of the garbage collector by calling get_roots function.
 *    2. Marks all the reachable objects by calling gc_mark function.
 *    3. Sweeps the memory and frees the unmarked objects by calling gc_sweep function.
 * 
 * 
 * Additions for Mark-Compact:
 * Before sweeping, we need compact the memory by calling the gc_compact function.
 * 
 */


void gc_run(){
    HashMap *roots = get_roots();
    if(!roots) return;
    print_linked_list();
    gc_mark(roots);
    gc_compact(roots); 
    gc_sweep();

    hashmap_free(roots);
    free(roots);
}

/* 
 * About this function:
 * 
 * I used this function for debugging purposes.
 * It dumps the current state of the garbage collector, i.e
 * for each address in the garbage collector's address set,
 * it prints the address, marked status, and size of the object.
 * 
 * This was very useful for debugging purposes.
 */

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

/* 
 * About this function:
 * 
 * This is yet another function that is accessible to the user.
 * basically its a wrapper around the malloc function.
 * 
 * we need to store the metadata for each object, so in our wrapper 
 * we will allocate memory for the object and also for the metadata.
 * and store the size and marked status in the metadata.
 * 
 * How it works:
 *     1. we allocate memory for the object 
 *     2. we also allocate memory for the metadata
 *     3. we initialize the metadata with marked = 0 and size = size of the object
 *     4. we insert the address of the object in the garbage collector's address set
 *     5. we insert the metadata in the hashmap with the address as the key
 * 
 * Additions for Mark-Compact:
 * In mark compact we update the linkedlist and count of total allocated objects.
 */

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
    metadata->address = address;
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


/* 
 * About this function:
 * 
 * This function frees the memory allocated for the object at the given address.
 * It also removes the address from the garbage collector's address set and
 * deletes the metadata associated with the object.
 * 
 * How it works:
 *     1. check if the address is NULL or not in the garbage collector's address set, if it is, return.
 *     3. get the metadata for the address from the hashmap.
 *     4. free the metadata.
 *     5. delete the address from the garbage collector's address set and hashmap.
 *     6. free the address.
 * 
 * Additions for Mark-Compact:
 * We will also remove the metadata from the linked list of metadata blocks.
 * This is done by iterating through the linked list. we also decrement the total_allocated count.
 */

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

/* used for debugging */
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


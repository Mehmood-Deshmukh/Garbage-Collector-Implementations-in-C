CC = gcc
CFLAGS = -Wall

IMPLEMENTATION=Mark-Compact
GC_SRC = ./src/$(IMPLEMENTATION)/gc.c
HASHMAP_SRC = ./src/HashMap-Implementation/hashmap.c
HASHSET_SRC = ./src/HashSet-Implementation/hashset.c
HASH_FUNCTIONS_SRC = ./src/Hash-Functions/hash_functions.c

GC_OBJ = gc.o
HASHMAP_OBJ = hashmap.o
HASHSET_OBJ = hashset.o
HASH_FUNCTIONS_OBJ = hash_functions.o


all: $(GC_OBJ) $(HASHMAP_OBJ) $(HASHSET_OBJ) $(HASH_FUNCTIONS_OBJ)


$(GC_OBJ): $(GC_SRC)
	$(CC) $(CFLAGS) -c $< -o $@


$(HASHMAP_OBJ): $(HASHMAP_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(HASHSET_OBJ): $(HASHSET_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(HASH_FUNCTIONS_OBJ): $(HASH_FUNCTIONS_SRC)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm *.o
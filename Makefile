CC = gcc
CFLAGS = -Wall

GC_MARK_AND_SWEEP_SRC = ./src/Mark-and-Sweep/gc.c
GC_MARK_COMPACT_SRC = ./src/Mark-Compact/gc.c
HASHMAP_SRC = ./src/HashMap-Implementation/hashmap.c
HASHSET_SRC = ./src/HashSet-Implementation/hashset.c
HASH_FUNCTIONS_SRC = ./src/Hash-Functions/hash_functions.c

GC_MARK_AND_SWEEP_OBJ = gc_mark_and_sweep.o
GC_MARK_COMPACT_OBJ = gc_mark_compact.o
HASHMAP_OBJ = hashmap.o
HASHSET_OBJ = hashset.o
HASH_FUNCTIONS_OBJ = hash_functions.o


all: $(GC_MARK_AND_SWEEP_OBJ) $(GC_MARK_COMPACT_OBJ) $(HASHMAP_OBJ) $(HASHSET_OBJ) $(HASH_FUNCTIONS_OBJ)


$(GC_MARK_AND_SWEEP_OBJ): $(GC_MARK_AND_SWEEP_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(GC_MARK_COMPACT_OBJ): $(GC_MARK_COMPACT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(HASHMAP_OBJ): $(HASHMAP_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(HASHSET_OBJ): $(HASHSET_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(HASH_FUNCTIONS_OBJ): $(HASH_FUNCTIONS_SRC)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm *.o
# Garbage Collector Implementations in C

This repository contains working implementations of two garbage collection algorithms:

1. **Mark and Sweep**: A traditional tracing garbage collection approach that marks live objects and sweeps away unreferenced ones.
2. **Mark and Compact**: An optimization that reduces memory fragmentation by relocating live objects to contiguous memory.

For a detailed explanation of the implementation and theory, please read the [documentation](./log.MD)

## Usage

### Step 1: Build the Object Files

First, create the object files using the provided Makefile:

```bash
make
```
**Note** : In the makefile, change the IMPLEMENTATION variable to either `mark_and_sweep` or `mark_and_compact` depending on which garbage collector implementation you want to use.

This will create the following object files:
- `gc.o`
- `hashmap.o` 
- `hashset.o`
- `hash_functions.o`

### Step 2: Compile Your Program

Once you have the object files, compile your program with them:

```bash
gcc your_program.c gc.o hashmap.o hashset.o hash_functions.o -I./src/(implemenation name) -o your_program
```
### Here is the complete set of commands to run the garbage collector:

```bash
# 1. Build the object files
make

# 2. Compile your program with the object files
gcc your_program.c gc.o hashmap.o hashset.o hash_functions.o -I./src/(implemenation name) -o your_program

# 3. Run your program
./your_program

# 4. Clean build files (optional)
make clean
```

### Basic Setup

```c
#include "gc.h"

int main() {
    /* Initialize the garbage collector */
    gc_init();
    
    /* Your program code here */
    
    /* Run the garbage collector */
    gc_run();
    return 0;
}
```

### Memory Allocation

Use `gc_malloc()` instead of regular `malloc()` to allocate memory that will be tracked by the garbage collector:

```c
size_t size = 1024;
void *test = gc_malloc(&size);

/* Memory will be automatically freed during garbage collection cycles */
```

### Manual Garbage Collection

You can manually trigger garbage collection:

```c
gc_run();
```

### Read this only if you are curious

Use `gc_dump()` to get information about the current state of the garbage collector:

```c
char *message = 'YOUR_HEADING' /* This is just a heading which will be printed before gc's state */
gc_dump(message);
```

### Memory Deallocation

If needed, you can manually free specific memory addresses:

```c
gc_free(&test);
```

### Example Program

```c
#include "gc.h"
#include <stdio.h>

int main() {
    gc_init();
    
    size_t size1 = 100;
    void *ptr1 = gc_malloc(&size1);
    
    size_t size2 = 200;
    void *ptr2 = gc_malloc(&size2);
    
    gc_run();
    

    /* checking gc's status */
    gc_dump("Garbage Collector State:");
    
    return 0;
}
```

## Contributing

Contributions are welcome! If you have any suggestions or improvements, feel free to open an issue or submit a pull request.

### Steps to contribute

1. Fork the repository
2. Create a new branch

```bash
git checkout -b feature/your-feature-name
```

3. Make your changes
4. Commit your changes

```bash
git commit -m "Add your commit message"
```

5. Push to the branch

```bash
git push origin feature/your-feature-name
```

6. Create a pull request
7. Wait for review and feedback
8. Make changes if required
9. Once approved, your changes will be merged into the main branch

## Acknowledgements

- [Writing a Mark-Sweep Garbage Collector](http://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/)
- [Tracing Garbage Collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)
- [Sameerkavthekar's Garbage Collector Implementation](https://github.com/sameerkavthekar/garbage-collector)
- [Mark-Compact Algorithm](https://en.wikipedia.org/wiki/Mark-compact_algorithm)
- [Mark-Sweep and Mark-Compact Algorithms (Tel Aviv University)](https://www.cs.tau.ac.il//~maon/teaching/2014-2015/seminar/seminar1415a-lec2-mark-sweep-mark-compact.pdf)
- [MurMur Hash (Payal Sankpal)](https://github.com/PayalSankpal/DSA-BloomFilters)
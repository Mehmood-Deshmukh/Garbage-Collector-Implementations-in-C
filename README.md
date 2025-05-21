# Garbage Collector Implementations in C

This repository contains working implementations of two garbage collection algorithms:

1. **Mark and Sweep**: A traditional tracing garbage collection approach that marks live objects and sweeps away unreferenced ones.
2. **Mark and Compact**: An optimization that reduces memory fragmentation by relocating live objects to contiguous memory.


For a detailed explanation of the implementation and theory, please read the [documentation](./log.MD)

## Acknowledgements

- [Writing a Mark-Sweep Garbage Collector](http://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/)
- [Tracing Garbage Collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)
- [Sameerkavthekar's Garbage Collector Implementation](https://github.com/sameerkavthekar/garbage-collector)
- [Mark-Compact Algorithm](https://en.wikipedia.org/wiki/Mark-compact_algorithm)
- [Mark-Sweep and Mark-Compact Algorithms (Tel Aviv University)](https://www.cs.tau.ac.il//~maon/teaching/2014-2015/seminar/seminar1415a-lec2-mark-sweep-mark-compact.pdf)
- [MurMur Hash (Payal Sankpal)](https://github.com/PayalSankpal/DSA-BloomFilters)

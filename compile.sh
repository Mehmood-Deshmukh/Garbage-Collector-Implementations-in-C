#!/bin/bash

INPUT_C_FILE=$1
IS_MARK_COMPACT=$2
OUTPUT_FILE=${INPUT_C_FILE%.c}

if [ -z "$INPUT_C_FILE" ]; then
  echo "Usage: $0 <input_c_file> < -c if you want to use mark-compact implementation>"
  exit 1
fi

if [ -z "$IS_MARK_COMPACT" ]; then
  IMPLEMENTATION_METHOD="mark_and_sweep"
else
  IMPLEMENTATION_METHOD="mark_compact"
fi

GC_MARK_AND_SWEEP_OBJ='gc_mark_and_sweep.o'
GC_MARK_COMPACT_OBJ='gc_mark_compact.o'
HASHMAP_OBJ='hashmap.o'
HASHSET_OBJ='hashset.o'
HASH_FUNCTIONS_OBJ='hash_functions.o'

make

if [ $? -ne 0 ]; then
  echo "Make failed. Please check your Makefile and source code."
  exit 1
fi

if [[ "$IMPLEMENTATION_METHOD" == "mark_and_sweep" ]]; then
  gcc -o "$OUTPUT_FILE" "$INPUT_C_FILE" "$GC_MARK_AND_SWEEP_OBJ" "$HASHMAP_OBJ" "$HASHSET_OBJ" "$HASH_FUNCTIONS_OBJ" -I./src/Mark-and-Sweep
elif [[ "$IMPLEMENTATION_METHOD" == "mark_compact" ]]; then
  gcc -o "$OUTPUT_FILE" "$INPUT_C_FILE" "$GC_MARK_COMPACT_OBJ" "$HASHMAP_OBJ" "$HASHSET_OBJ" "$HASH_FUNCTIONS_OBJ" -I./src/Mark-Compact
else
  echo "Invalid implementation method. Use 'mark_and_sweep' or 'mark_compact'."
  exit 1
fi

if [ $? -ne 0 ]; then
  echo "Compilation failed. Please check your source code."
  exit 1
fi

echo "Compilation successful. Output file: $OUTPUT_FILE"

make clean
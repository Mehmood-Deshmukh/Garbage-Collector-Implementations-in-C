#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

int main(){
    gc_init();
    int *ptr = (int *)gc_malloc(sizeof(int) * 1024);

    ptr = NULL; // Simulating a null pointer to test gc_free
    gc_run();
}
#include<stdio.h>
#include<stdlib.h>
#include "gc.h"

typedef struct Node {
    struct Node *left;
    struct Node *right;
} Node;


int main(){
    gc_init();
    Node *a = (Node *)gc_malloc(sizeof(Node));
    Node *b = (Node *)gc_malloc(sizeof(Node));
    
    gc_dump("Allocated Graph");
    a = NULL;
    gc_run();
    gc_dump("After GC");
    return 0;
}

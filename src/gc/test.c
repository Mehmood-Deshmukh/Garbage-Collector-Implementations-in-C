#include<stdio.h>
#include<stdlib.h>
#include "gc.h"

typedef struct Node {
    int data;
    struct Node *next;
} Node;


int main(){
    gc_init();
    Node *a = (Node *)gc_malloc(sizeof(Node));
    Node *b = (Node *)gc_malloc(sizeof(Node));
    Node *c = (Node *)gc_malloc(sizeof(Node));

    b->next = c;
    
    b = NULL;
    c = NULL;
    gc_dump("Allocated List");
    gc_run();
    gc_dump("After GC");
    return 0;
}

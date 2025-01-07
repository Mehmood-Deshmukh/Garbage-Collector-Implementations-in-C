#include<stdio.h>
#include<stdlib.h>
#include "gc.h"

typedef struct Node{
  int data;
  struct Node *left;
  struct Node *right;
} Node;

int main(){
    gc_init();
    Node *a = (Node *)gc_malloc(sizeof(Node));
    Node *b = (Node *)gc_malloc(sizeof(Node));
    Node *c = (Node *)gc_malloc(sizeof(Node));
    Node *d = (Node *)gc_malloc(sizeof(Node));
    Node *e = (Node *)gc_malloc(sizeof(Node));

    a->left = b;
    a->right = c;

    c->left = d;
    c->right = e;

    b = c = d = e = NULL;
    gc_dump("Allocated Stuff");
    a->right = NULL;
    gc_run();
    gc_dump("After GC");
    return 0;
}

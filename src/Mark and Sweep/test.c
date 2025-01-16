#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

typedef struct Node{
  int data;
  struct Node *left, *right;
} Node;

Node *create_graph(){
  Node *a = (Node *)gc_malloc(sizeof(Node));
  Node *b = (Node *)gc_malloc(sizeof(Node));
  Node *c = (Node *)gc_malloc(sizeof(Node));
  Node *d = (Node *)gc_malloc(sizeof(Node));
  Node *e = (Node *)gc_malloc(sizeof(Node));
  Node *f = (Node *)gc_malloc(sizeof(Node));
  Node *g = (Node *)gc_malloc(sizeof(Node));
  Node *h = (Node *)gc_malloc(sizeof(Node));

  a->left = b;
  a->right = c;

  c->left = d;
  c->right = e;

  e->left = f;
  e->right = g;

  g->right = h;

  return a;
}

int main(){
  gc_init();

  Node *a = create_graph();

  gc_dump("Allocated Graph");
  a->right = NULL;
  gc_run();
  gc_dump("After GC");
  return 0;
}

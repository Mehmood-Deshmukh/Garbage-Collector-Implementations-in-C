#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

typedef struct Node{
  char data;
  struct Node *left, *right;
} Node;

Node *createGraph(){
  Node *a = (Node *)gc_malloc(sizeof(Node));
  Node *b = (Node *)gc_malloc(sizeof(Node));
  Node *c = (Node *)gc_malloc(sizeof(Node));
  Node *d = (Node *)gc_malloc(sizeof(Node));
  Node *e = (Node *)gc_malloc(sizeof(Node));
  Node *f = (Node *)gc_malloc(sizeof(Node));
  Node *g = (Node *)gc_malloc(sizeof(Node));
  Node *h = (Node *)gc_malloc(sizeof(Node));

  a->data = 'a';
  b->data = 'b';
  c->data = 'c';
  d->data = 'd';
  e->data = 'e';
  f->data = 'f';
  g->data = 'g';
  h->data = 'h';

  a->left = b;
  a->right = c;

  c->right = e;

  e->left = f;

  return a;
}

int main(){
  gc_init();

  Node *a = createGraph();

  gc_dump("Allocated Graph");
  a->left = NULL;
  gc_run();
  gc_dump("After GC");

  printf("%c\n", a->data);
  printf("%c\n", a->right->data);
  printf("%c\n", a->right->right->data);
  printf("%c\n", a->right->right->left->data);
  return 0;
}

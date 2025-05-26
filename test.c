#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

  typedef struct Node{
    char data;
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
    c->left = d;

    e->left = f;
    e->right = g;

    g->right = h;

    return a;
  }

void print_graph(Node *root){
  if(!root) return;

  printf("%c\n", root->data);
  print_graph(root->left);
  print_graph(root->right);
}

int main(){
  gc_init();

  Node *a = create_graph();

  gc_dump("Allocated Graph");
  a->left = NULL;
  gc_run();
  gc_dump("After GC");

  print_graph(a);
  return 0;
}

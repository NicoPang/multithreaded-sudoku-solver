#include <stdio.h>
#include <stdlib.h>
#include "cells.h"
#include "trail.h"

// Basic Cell Functions

Cell* makeCell(int s, int id) {
  Cell* c = (Cell*)malloc(sizeof(Cell));
  int* guesses = (int*)malloc(sizeof(int) * (s + 1));
  c->id = id;
  c->val = 0;
  c->ngs = s;
  c->gs = guesses;
  for (int i = 1; i <= s; i++)
    c->gs[i] = 1;
  return c;
}

void freeCell(Cell* c) {
  free(c->gs);
  free(c);
}

void setValue(Cell* c, int v, int sz) {
  c->val = v;
  c->ngs = 0;
  for (int i = 1; i <= sz; i++)
    c->gs[i] = 0;
} 

int removeGuess(Cell* c, int n) {
  if (c->gs[n] == 1) {
    c->gs[n] = 0;
    c->ngs--;
  }
  return c->ngs;
}

void printCell(Cell* c, int size) {
  printf("Cell ID: %d with %d Guesses: { \n", c->id, c->ngs);
  for (int i = 1; i <= size; i++) {
    printf("%d ", c->gs[i]);
  }
  printf("}\n");
}

Cell* copyCell(Cell* orig, int s) {
  Cell* new = (Cell*)malloc(sizeof(Cell));
  int* guesses = (int*)malloc(sizeof(int) * (s + 1));
  new->id = orig->id;
  new->val = orig->val;
  new->ngs = orig->ngs;
  new->gs = guesses;
  for (int i = 1; i <= s; i++)
    new->gs[i] = orig->gs[i];
  return new;
}

// Basic Group Functions

Group* makeGroup(int max) {
  Group* g = (Group*)malloc(sizeof(Group));
  Cell** cells = (Cell**)malloc(sizeof(Cell*) * max);
  g->max = max;
  g->ncs = 0;
  g->cs = cells;
  return g;
}

int addCell(Group* g, Cell* c) {
  if (g->ncs == g->max) {
    printf("Error: cannot add more cells to group.\n");
    return 0;
  }
  g->cs[g->ncs++] = c;
  return 1;
}

void freeGroup(Group* g) {
  free(g->cs);
  free(g);
}


#include <stdio.h>
#include <stdlib.h>
#include "cells.h"
#include "trail.h"

// Data creation
Data makeData(int type, int ID, int v) {
  Data d = {type, ID, v};
  return d;
}

void printData(Data d) {
  printf("{%d %d %d}\n", d.type, d.cellID, d.value);
}

// Trail functions

Trail* makeTrail() {
  Trail* t = (Trail*)malloc(sizeof(Trail));
  t->sz = 0;
  t->max = MAX_CHANGES;
  t->changes = (Data*)malloc(sizeof(Data) * t->max);
  return t;
}

Trail* reallocTrail(Trail* t) {
  t->max *= 2;
  t->changes = (Data*)realloc(t->changes, t->max);
  return t;
}

void freeTrail(Trail* t) {
  free(t->changes);
  free(t);
}

void makeChange(Trail* t, int type, int ID, int v) {
  if (t->sz == t->max) {
    reallocTrail(t);
  }
  Data data = makeData(type, ID, v);
  //printf("Saving data: ");
  //printData(data);
  t->changes[t->sz++] = data;
}

Data extractChange(Trail* t) {
  Data data = t->changes[t->sz - 1];
  //printf("Restoring data: ");
  //printData(data);
  t->sz--;
  return data;
}

// Cell functions with Trail
void setValueT(Cell* c, int v, int sz, Trail* t) {
  if (t != NULL) {
    for (int i = 1; i <= sz; i++) {
      if (c->gs[i] == 1) {
	makeChange(t, 0, c->id, i);
      }
    }
    makeChange(t, 1, c->id, v);
  }
  setValue(c, v, sz);
} 

int removeGuessT(Cell* c, int n, Trail* t) {
  if (t != NULL && c->gs[n] == 1) {
    makeChange(t, 0, c->id, n);
  }

  return removeGuess(c, n);
}

// Mark Functions

Marks* createMarks() {
  Marks* m = (Marks*)malloc(sizeof(Marks));
  m->sz = 0;
  m->max = 1;
  m->marks = (int*)malloc(sizeof(int) * m->max);
  return m;
}

Marks* reallocMarks(Marks* m) {
  m->max *= 2;
  m->marks = (int*)realloc(m->marks, sizeof(int) * m->max);
  return m;
}

void freeMarks(Marks* m) {
  free(m->marks);
  free(m);
}

void addMark(Marks* m, int index) {
  if (m->sz == m->max) {
    reallocMarks(m);
  }
  m->marks[m->sz++] = index;
}

int extractMark(Marks* m) {
  int mark = m->marks[m->sz - 1];
  m->sz--;
  return mark;
}

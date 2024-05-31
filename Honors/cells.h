#ifndef CELLS_H
#define CELLS_H

typedef struct Cell {
  int id;
  int val;
  int ngs;
  int* gs;
} Cell;

typedef struct Group {
  int max;
  int ncs;
  Cell** cs;
} Group;

// Basic Cell Functions
Cell* makeCell(int s, int id);
void freeCell(Cell* c);
void setValue(Cell* c, int v, int sz);
int removeGuess(Cell* c, int n);
void printCell(Cell* c, int size);
Cell* copyCell(Cell* orig, int s);
// Basic Group Functions
Group* makeGroup(int max);
int addCell(Group* g, Cell* c);
void freeGroup(Group* g);

#endif

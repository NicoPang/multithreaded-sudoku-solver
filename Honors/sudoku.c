#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cells.h"
#include "trail.h"
#include "sudoku.h"

// Basic Sudoku Functions
Sudoku* makeSudoku(int size) {
  Sudoku* s = (Sudoku*)malloc(sizeof(Sudoku));
  Cell** cells = (Cell**)malloc(sizeof(Cell*) * size * size);
  s->sz = size;
  s->rem = size * size;
  s->cs = cells;

  for (int i = 0; i < size * size; i++)
    s->cs[i] = makeCell(size, i);
  return s;
}

void freeSudoku(Sudoku* s) {
  for (int i = 0; i < s->sz * s->sz; i++)
    freeCell(s->cs[i]);
  free(s->cs);
  free(s);
}

int removeGuesses(int guess, Group* g, int ignore, Trail* t) {
  int violations = 0;
  for (int i = 0; i < g->ncs; i++) {
    if (g->cs[i]->id != ignore && removeGuessT(g->cs[i], guess, t) == 0) {
      violations++;
    }
  }
  return violations;
}

int setCell(Sudoku* s, int v, int r, int c, Trail* t) {
  Cell* cell = s->cs[getID(r, c, s->sz)];
  if (cell->val > 0)
    return 0;
  if (cell->gs[v] == 0)
    return -1;
  Group* row = getFilteredRow(s, r);
  Group* col = getFilteredCol(s, c);
  Group* box = getFilteredBox(s, getBoxByID(cell->id, s->sz));
  int rowV = removeGuesses(v, row, cell->id, t);
  freeGroup(row);
  int colV = removeGuesses(v, col, cell->id, t);
  freeGroup(col);
  int boxV = removeGuesses(v, box, cell->id, t);
  freeGroup(box);
  setValueT(cell, v, s->sz, t);
  s->rem--;
  if (rowV == 0 && colV == 0 && boxV == 0)
    return 1;
  else
    return -2;
}

int setCellByID(Sudoku* s, int v, int id, Trail* t) {
  return setCell(s, v, getRowByID(id, s->sz), getColByID(id, s->sz), t);
}

int isSolved(Sudoku* s) {
  return s->rem == 0;
}

Sudoku* copySudoku(Sudoku* orig) {
  Sudoku* new = (Sudoku*)malloc(sizeof(Sudoku));
  Cell** cells = (Cell**)malloc(sizeof(Cell*) * orig->sz * orig->sz);
  new->sz = orig->sz;
  new->rem = orig->rem;
  new->cs = cells;

  for (int i = 0; i < new->sz * new->sz; i++)
    new->cs[i] = copyCell(orig->cs[i], new->sz);
  return new;
}

// Sudoku sectioning

Group* getRow(Sudoku* s, int r) {
  Group* row = makeGroup(s->sz);
  for (int i = 0; i < s->sz; i++)
    addCell(row, s->cs[getID(r, i, s->sz)]);
  return row; 
}

Group* getFilteredRow(Sudoku* s, int r) {
  Group* row = makeGroup(s->sz);
  for (int i = 0; i < s->sz; i++) {
    Cell* cell = s->cs[getID(r, i, s->sz)];
    if (cell->val == 0) {
      addCell(row, cell);
    }  
  }
  return row; 
}

Group* getCol(Sudoku* s, int c) {
  Group* col = makeGroup(s->sz);
  for (int i = 0; i < s->sz; i++)
    addCell(col, s->cs[getID(i, c, s->sz)]);
  return col;
}

Group* getFilteredCol(Sudoku* s, int c) {
  Group* col = makeGroup(s->sz);
  for (int i = 0; i < s->sz; i++) {
    Cell* cell = s->cs[getID(i, c, s->sz)];
    if (cell->val == 0) {
      addCell(col, cell);
    }  
  }
  return col;

}

Group* getBox(Sudoku* s, int bxID) {
  int root = (int)sqrt(s->sz);
  Group* box = makeGroup(s->sz);
  int bxRow = (bxID - (bxID % root))/root;
  int rowSt = bxRow * root;
  int bxCol = bxID % root;
  int colSt = bxCol * root;
  for (int r = rowSt; r < rowSt + root; r++) {
    for (int c = colSt; c < colSt + root; c++) {
      addCell(box, s->cs[getID(r, c, s->sz)]);
    }
  } 
  return box;
}

Group* getFilteredBox(Sudoku* s, int bxID) {
  int root = (int)sqrt(s->sz);
  Group* box = makeGroup(s->sz);
  int bxRow = (bxID - (bxID % root))/root;
  int rowSt = bxRow * root;
  int bxCol = bxID % root;
  int colSt = bxCol * root;
  for (int r = rowSt; r < rowSt + root; r++) {
    for (int c = colSt; c < colSt + root; c++) {
      Cell* cell = s->cs[getID(r, c, s->sz)];
      if (cell->val == 0) {
	addCell(box, cell);
      }
    }
  } 
  return box;
}

// Sudoku helper functions

int getID(int r, int c, int sz) {
  return r * sz + c;
}
int getRowByID(int id, int sz) {
  return id / sz;
}
int getColByID(int id, int sz) {
  return id % sz;
}
int getBoxByID(int id, int sz) {
  int root = (int)sqrt(sz);
  int bxRow = id / sz / root;
  int bxCol = id % sz / root;
  return bxRow * root + bxCol;
}

// Sudoku printing

void printSudoku(Sudoku* s) {
  int root = (int)sqrt(s->sz);

  int row = 0;
  for (int i = 0; i < root; i++) {
    printDivider(s->sz);
    for (int j = 0; j < root; j++) {
      printRow(s, row);
      row++;
    }
  }
  printDivider(s->sz);
}


void printRow(Sudoku* s, int r) {
  Group* row = getRow(s, r);
  int root = (int)sqrt(s->sz);

  printf("|");
  int col = 0;
  for (int i = 0; i < root; i++) {
    for (int j = 0; j < root; j++) {
      printf("|");
      if (row->cs[col]->ngs != 0) 
	printf("  ");
      else if (row->cs[col]->val == 0)
	printf(" ?");
      else
	printf("%2d", row->cs[col]->val); 
      col++;     
    }
    printf("|");
  }
  printf("|\n");
  freeGroup(row);
}


void printDivider(int size) {
  int root = (int)sqrt(size);
  for (int i = 0; i < root; i++) {
    printf("[]");
    for (int j = 0; j < 3 * root - 1; j++)
      printf("=");
  }
  printf("[]\n");
}

// Sudoku importing
Sudoku* importSudoku(char* path, int sz) {
  // Setup Path
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    return NULL;
  }
  Sudoku* s = makeSudoku(sz);
  char* line = (char*)malloc(sizeof(char) * 10);
  int max = 10;
  while ((line = fgets(line, max, file)) != NULL) {
    int row, col, val;
    if (sscanf(line, "%d %d %d\n", &row, &col, &val) == 3) {
      if (row < 1 || row > 9 || col < 1 || col > 9 || val < 1 || val > 9) {
	printf("INVALID: Unacceptable values for <row> <col> <val> -> %s", line);
      } else {
	int error;
	error = setCell(s, val, row - 1, col - 1, NULL);
	switch (error) {
	case -2 :
	  printf("WARNING: Setting (%d, %d) to %d leads to an impossible sudoku.\n", row, col, val);
	  break;
	case -1 :
	  printf("INVALID: Setting (%d, %d) to %d is impossible due to an immediate violation of sudoku rules.\n", row, col, val);
	  break;
	case 0:
	  printf("WARNING: (%d, %d) not set to %d due to there already being a value there.\n", row, col, val);
	default:
	  printf("SUCCESS: (%d, %d) set to %d\n", row, col, val);
	}
      }
    } else
      printf("INVALID: Unacceptable line -> %s", line);
  }
  free(line);
  return s;
}
Sudoku* createSudoku(int sz);


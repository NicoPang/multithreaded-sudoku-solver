#ifndef SUDOKU_H
#define SUDOKU_H

#define IMPORT 0
#define CREATE 1

typedef struct Sudoku {
  int sz;
  int rem;
  Cell** cs;
} Sudoku;

// Basic Sudoku Functions
Sudoku* makeSudoku(int size);
void freeSudoku(Sudoku* s);
int removeGuesses(int guess, Group* g, int ignore, Trail* t);
int setCell(Sudoku* s, int v, int r, int c, Trail* t);
int setCellByID(Sudoku* s, int v, int id, Trail* t);
int isSolved(Sudoku* s);
Sudoku* copySudoku(Sudoku* orig);

// Sudoku Sectioning

Group* getRow(Sudoku* s, int r);
Group* getFilteredRow(Sudoku* s, int r);
Group* getCol(Sudoku* s, int c);
Group* getFilteredCol(Sudoku* s, int c);
Group* getBox(Sudoku* s, int bxID);
Group* getFilteredBox(Sudoku* s, int bxID);

// Sudoku Helper Functions
int getID(int r, int c, int sz);
int getRowByID(int id, int sz);
int getColByID(int id, int sz);
int getBoxByID(int id, int sz);

// Sudoku Printing
void printSudoku(Sudoku* s);
void printRow(Sudoku* s, int r);
void printDivider(int size);

// Sudoku Importing
Sudoku* importSudoku(char* path, int sz);
Sudoku* createSudoku(int sz);

#endif

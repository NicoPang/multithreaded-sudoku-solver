#ifndef TRAIL_H
#define TRAIL_H

#define GUESS 0
#define VALUE 1

#define MAX_CHANGES 1000

typedef struct Data {
  int type;
  int cellID;
  int value;
} Data;

typedef struct Trail {
  int sz;
  int max;
  Data* changes;
} Trail;

typedef struct Marks {
  int sz;
  int max;
  int* marks;
} Marks;

Data makeData(int type, int ID, int v);

Trail* makeTrail();
Trail* reallocTrail(Trail* t);
void freeTrail(Trail* t);
void makeChange(Trail* t, int type, int ID, int v);
Data extractChange(Trail* t);

void setValueT(Cell* c, int v, int sz, Trail* t);
int removeGuessT(Cell* c, int n, Trail* t);

Marks* createMarks();
Marks* reallocMarks(Marks* m);
void freeMarks(Marks* m);
void addMark(Marks* m, int index);
int extractMark(Marks* m);

#endif

#ifndef SOLVER_H
#define SOLVER_H

typedef struct Solutions {
  int numSols;
  int maxSols;
  Sudoku** solutions;
} Solutions;

typedef struct Job {
  Sudoku* s;
  int ngs;
} Job;

typedef struct SharedInfo {
  int stillBranching;
  int numJobs;
  int maxJobs;
  Job* jobs;
  int waitThreads;
  int numThreads;
  Solutions* solutions;
  pthread_mutex_t mtx;
  pthread_cond_t done;
} SharedInfo;

typedef struct ThreadInfo {
  Job* job;
  Trail* t;
  Marks* m;
  SharedInfo* SI;
  pthread_t name;
} ThreadInfo;

typedef struct TBInfo {
  int maxDepth;
  Sudoku* s;
  Trail* t;
  Marks* m;
  SharedInfo* SI;
  pthread_t name;
} TBInfo;

// Sudoku Scanning
int findSingleton(Cell* c, int sz);
int findSingletons(Sudoku* s, Trail* t);
int findHiddenSinglesGroup(Sudoku* s, Group* g, Trail* t);
int findHiddenSingles(Sudoku* s, Trail* t);
int findPreemptiveSetAux(Sudoku* s, Group* g, int curr, int noin, int inIDs[], int nogs, int gs[], Trail* t);
int findPreemptiveSet(Sudoku* s, Group* g, Trail* t);
int findPreemptiveSets(Sudoku* s, Trail* t);
int scanSudoku(Sudoku* s, Trail* t);

// Sudoku Guessing
void makeGuess(Marks* m, Trail* t, Sudoku* s, int ID, int guess);
int findGuessCell(Sudoku* s);
int findGuess(Cell* c, int sz);
void restore(Marks* m, Trail* t, Sudoku* s);
int chainRestore(Marks* m, Trail* t, Sudoku* s, int undos);

//Sudoku Solving
void solveSudoku(Sudoku* s);

#endif


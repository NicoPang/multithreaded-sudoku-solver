#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "cells.h"
#include "trail.h"
#include "sudoku.h"
#include "solver.h"

// Sudoku Scanning

int findSingleton(Cell* c, int sz) {
  // Assumes singleton already exists
  for (int i = 1; i <= sz; i++) {
    if (c->gs[i] == 1) {
      return i;
    }
  }
  //printf("FATAL ERROR at: findSingleton() for cell %d", c->id);
  return -1;
}

int findSingletons(Sudoku* s, Trail* t) {
  int singletons = 0;
  for (int i = 0; i < s->sz * s->sz; i++) {
    if (s->cs[i]->val == 0 && s->cs[i]->ngs == 1) {
      singletons++;
      int digit = findSingleton(s->cs[i], s->sz);
      int error;

      if (digit == -1)
	return -2;
      error = setCellByID(s, digit, i, t);
      switch (error) {
      case 1: 
	break;
      case -2:
	return -1;
      default:
	//printf("FATAL ERROR at: findSingletons() due to impossible error with id %d\n", error);
	return -1;
      }
    }
  }
  return singletons;
}
int findHiddenSinglesGroup(Sudoku* s, Group* g, Trail* t) {
  // -1 not found, >=0 sole instance, -2 multiple instances
  int firstInstances[s->sz + 1];
  for (int k = 1; k <= s->sz; k++)
    firstInstances[k] = -1;
  for (int i = 0; i < g->ncs; i++) {
    Cell* c = g->cs[i];
    for (int j = 1; j <= s->sz; j++) {
      if (c->gs[j] == 1) {
	if (firstInstances[j] == -1) {
	  firstInstances[j] = c->id;
	} else if (firstInstances[j] >= 0) {
	  firstInstances[j] = -2;
	}
      }
    }
  }

  int noHS = 0;
  int error;

  for (int m = 1; m <= s->sz; m++) {
    if (firstInstances[m] >= 0) {
      noHS++;
      error = setCellByID(s, m, firstInstances[m], t);
      if (error != 1) {
	return -1;
      }
    }
  }
  return noHS;
}
int findHiddenSingles(Sudoku* s, Trail* t) {
  int noHS = 0;
  for (int i = 0; i < s->sz; i++) {
    Group* row = getFilteredRow(s, i);
    Group* col = getFilteredCol(s, i);
    Group* box = getFilteredBox(s, i);

    int rowHS = findHiddenSinglesGroup(s, row, t);
    free(row);
    int colHS = findHiddenSinglesGroup(s, col, t);
    free(col);
    int boxHS = findHiddenSinglesGroup(s, box, t);
    free(box);

    if (rowHS == -1 || colHS == -1 || boxHS == -1)
      return -1;
    noHS += rowHS + colHS + boxHS; 
  }
  return noHS;
}

int findPreemptiveSetAux(Sudoku* s, Group* g, int curr, int noin, int inIDs[], int nogs, int gs[], Trail* t) {
  if (curr == g->ncs) {
    return 0;
  }

  Cell* c = g->cs[curr];
  int newnoin = noin;
  int newinIDs[g->ncs];
  for (int x = 0; x < g->ncs; x++)
    newinIDs[x] = inIDs[x];
  int newnogs = nogs;
  int newgs[s->sz + 1];
  for (int y = 1; y <= s->sz; y++)
    newgs[y] = gs[y]; 

  newnoin++;
  newinIDs[curr] = 1;
  for (int i = 1; i <= s->sz; i++) {
    if (c->gs[i] == 1 && newgs[i] == 0) {
      newgs[i] = 1;
      newnogs++;
    }
  }
  if (newnogs < newnoin) {
    //printf("Error: number of guesses cannot be smaller than cells\n");
    return -1;
  } else if (newnoin == newnogs) {
    int removed = 0;
    for (int j = 0; j < g->ncs; j++) {
      for (int k = 1; k <= s->sz; k++) {
	if (newinIDs[j] == 0 && newgs[k] == 1 && g->cs[j]->gs[k] == 1) {
	  removed++;
	  int remgs = removeGuessT(g->cs[j], k, t);
	  if (remgs == 0) {
	    //printf("Error: no more remaining guesses for cell with id %d\n", g->cs[j]->id);
	    return -1;
	  }
	}
      }
    }
    return removed;
  } else {
    int include = findPreemptiveSetAux(s, g, curr + 1, newnoin, newinIDs, newnogs, newgs, t);
    int exclude = findPreemptiveSetAux(s, g, curr + 1, noin, inIDs, nogs, gs, t);

    if (include == -1 || exclude == -1)
      return -1;
    return include + exclude;
  }
}

int findPreemptiveSet(Sudoku* s, Group* g, Trail* t) {
  int inIDs[g->ncs];
  for (int i = 0; i < g->ncs; i++)
    inIDs[i] = 0;
  int gs[s->sz + 1];
  for (int j = 1; j <= s->sz; j++)
    gs[j] = 0; 
  return findPreemptiveSetAux(s, g, 0, 0, inIDs, 0, gs, t);
}

int findPreemptiveSets(Sudoku* s, Trail* t) {
  int removed = 0;
  for (int i = 0; i < s->sz; i++) {
    Group* row = getFilteredRow(s, i);
    Group* col = getFilteredCol(s, i);
    Group* box = getFilteredBox(s, i);

    int rowR = findPreemptiveSet(s, row, t);
    free(row);
    int colR = findPreemptiveSet(s, col, t);
    free(col);
    int boxR = findPreemptiveSet(s, box, t);
    free(box);
    if (rowR == -1 || colR == -1 || boxR == -1)
      return -1;  
    removed += rowR + colR + boxR; 
  }
  return removed;  
}

int scanSudoku(Sudoku* s, Trail* t) {
  //printf("Starting new scan!\n");
  int singletons = 0, HS = 0, removed = 0;
  do {
    //printf("SINGLETONS\n");
    singletons = findSingletons(s, t);
    //printf("HIDDEN SINGLES\n");
    HS = findHiddenSingles(s, t);
    //printf("PREEMPTIVE SETS\n");
    removed = findPreemptiveSets(s, t);
    //printSudoku(s);
    //printf("singletons: %d HS: %d removed: %d\n", singletons, HS, removed);
  } while ((singletons > 0 || HS > 0 || removed > 0) && (singletons != -1 && HS != -1 && removed != -1));

  if (singletons < 0 || HS < 0 || removed < 0) {
    return -1;
  } else
    return 0;
}

// Sudoku Guessing
void makeGuess(Marks* m, Trail* t, Sudoku* s, int ID, int guess) {
  //printf("Making guess %d for cell %d\n", guess, ID);
  //printCell(s->cs[ID], s->sz);
  addMark(m, t->sz);
  setCellByID(s, guess, ID, t);
}

int findGuessCell(Sudoku* s) {
  for(int i = 0; i < s->sz * s->sz; i++) {
    if (s->cs[i]->ngs > 0) {
      return i;
    }
  }
  return -1;
}

int findGuess(Cell* c, int sz) {
  for (int i = 1; i <= sz; i++) {
    if (c->gs[i] == 1) {
      return i;
    }
  }
  return -1;
}

void restore(Marks* m, Trail* t, Sudoku* s) {
  int mark = extractMark(m);
  while (t->sz != mark) {
    Data change = extractChange(t);
    if (change.type == 1) {
      s->cs[change.cellID]->val = 0;
      s->rem++;
    } else {
      if (s->cs[change.cellID]->gs[change.value] == 0) {
	s->cs[change.cellID]->gs[change.value] = 1;
	s->cs[change.cellID]->ngs++;
      }
    }
  }
}

int chainRestore(Marks* m, Trail* t, Sudoku* s, int undos) {
  //printf("Starting chain restore.\n");
  //printf("ORIGINAL:\n");
  //printSudoku(s);
  restore(m, t, s);
  //printf("RESTORED:\n");
  //printSudoku(s);

  int guessID = findGuessCell(s);
  if (guessID == -1) {
    return -1;
  }
  int guess = findGuess(s->cs[guessID], s->sz);
  //printCell(s->cs[guessID], s->sz);
  if (s->cs[guessID]->ngs == 1) {
    if (m->sz == 0) {
      return -1;
    } else {
      return chainRestore(m, t, s, undos + 1);
    }
  } else {
    //printf("removing guess %d\n", guess);
    if (m->sz == 0) {
      removeGuessT(s->cs[guessID], guess, NULL);
      return undos;
    }
    removeGuessT(s->cs[guessID], guess, t);
    return undos;
  }
}

// Sudoku Solving

void solveSudoku(Sudoku* s) {
  Trail* t = makeTrail();
  Marks* m = createMarks();

  int scanEr, restEr, solutions = 0;
  scanEr = scanSudoku(s, NULL);
  //printSudoku(s);
  if (scanEr == -1) {
    //printf("Sudoku cannot be solved.\n");
    freeTrail(t);
    freeMarks(m);
    return;
  }

  if (isSolved(s)) {
    solutions++;
    //printf("Solution:\n");
    //printSudoku(s);
    printf("There were %d solutions found.\n", solutions);
    freeTrail(t);
    freeMarks(m);
    return;
  }

  // At this point, guessing is required.
  while (1) {
    int guessID = findGuessCell(s);
    int guess = findGuess(s->cs[guessID], s->sz);
    makeGuess(m, t, s, guessID, guess);

    scanEr = scanSudoku(s, t);
    if (scanEr == -1) {
      //printf("Scanning resulted in an error:\n");
      //printSudoku(s);
      restEr = chainRestore(m, t, s, 1);
      if (restEr == -1) {
	break;
      }
    } else if (isSolved(s)) {
      solutions++;
      //printf("Solution:\n");
      //printSudoku(s);
      restEr = chainRestore(m, t, s, 1);
      if (restEr == -1) {
	break;
      }
    }
  }
  printf("There were %d solutions found.\n", solutions);
}

// Thread Object Manipulation

Solutions* makeSStack() {
  Solutions* s = (Solutions*)malloc(sizeof(Solutions));
  s->numSols = 0;
  s->maxSols = 1;

  Sudoku** solutions = (Sudoku**)malloc(sizeof(Sudoku*) * s->maxSols);
  s->solutions = solutions;
  return s; 
}

void reallocSStack(Solutions* s) {
  s->maxSols *= 2;
  s->solutions = (Sudoku**)realloc(s->solutions, sizeof(Sudoku*) * s->maxSols);
}

void freeSStack(Solutions* s) {
  for (int i = 0; i < s->numSols; i++) {
    freeSudoku(s->solutions[i]);
  }
  free(s->solutions);
  free(s);
}

// Threading

void* trailBlaze(void* args) {
  // Extract relevant information from info
  TBInfo* info = args;
  Sudoku* s = info->s;
  Trail* t = info->t;
  Marks* m = info->m;
  SharedInfo* shr = info->SI;

  //printf("Trailblazer has extracted info.\n");
  
  // Create necessary vars
  int guesses = 0;
  int restEr, scanEr;

  // Create jobs
  while (1) {
    // Undo if max guesses reached
    if (guesses == info->maxDepth) {
      // Add current state to jobs
      Sudoku* copy = copySudoku(s);
      Job j = {copy, guesses};
      // TODO: Concurrency stuff; add job to joblist
      // - Obtain lock
      // - insert job
      // - increase numJobs
      // - wake another guy up
      // - Unlock
      pthread_mutex_lock(&shr->mtx);
      shr->jobs[shr->numJobs++] = j;
      pthread_cond_signal(&shr->done);
      pthread_mutex_unlock(&shr->mtx);

      restEr = chainRestore(m, t, s, 1);
      if (restEr == -1) {
	// No more jobs.
	break;
      } else {
	guesses -= restEr;
      }
    }
    int guessID = findGuessCell(s);
    int guess = findGuess(s->cs[guessID], s->sz);
    makeGuess(m, t, s, guessID, guess);
    guesses++;

    scanEr = scanSudoku(s, t);
    if (scanEr == -1) {
      restEr = chainRestore(m, t, s, 1);
      if (restEr == -1) {
	break;
      } else {
	guesses -= restEr;
      }
    } else if (isSolved(s)) {
      // TODO: Concurrency add solution to shared info
      // - Obtain lock
      // - Increase num solutions
      // - Copy solution to Solution stack
      // - Unlock
      //printf("Found a solution!\n");
      pthread_mutex_lock(&shr->mtx);
      if (shr->solutions->numSols == shr->solutions->maxSols) {
	reallocSStack(shr->solutions);
      }
      shr->solutions->solutions[shr->solutions->numSols++] = copySudoku(s);
      pthread_mutex_unlock(&shr->mtx);
      //printf("Successfully added solution!\n");
      restEr = chainRestore(m, t, s, 1);
      if (restEr == -1) {
	break;
      } else {
	guesses -= restEr;
      }
    } 
  }
  
  // Done with jobs
  // TODO: Concurrency end trailblazing phase
  // - Obtain lock
  // - Turn off isBranching
  // - Unlock
  pthread_mutex_lock(&shr->mtx);
  shr->stillBranching = 0;
  pthread_mutex_unlock(&shr->mtx);
  freeMarks(m);
  freeTrail(t);
}

void* solveThread(void* args) {
  // Unpack info
  ThreadInfo* info = args;
  Trail* t = info->t;
  Marks* m = info->m;
  SharedInfo* shr = info->SI;
  Job* job = info->job;
  Sudoku* s;
  int sol = 0, jobs = 0;
  while (1) {
    // Get Lock
    // Step 1: Look for job
    // - Increase waitThreads
    // - While isBranching && jobs == 0 
    // - Wait
    // - If waitThreads == numThreads && jobs == 0
    // - Unlock
    // - Break
    // - Else
    // - Grab job
    // - Decrease jobs and waitThreads by 1
    pthread_mutex_lock(&shr->mtx);
    shr->waitThreads++;
    while (shr->stillBranching && shr->numJobs == 0) {
      //printf("Still Branching: %d\n", shr->stillBranching);
      pthread_cond_wait(&shr->done, &shr->mtx);
    }
    //printf("Testing...\n");
    if (shr->numJobs == 0) {
      if (s != NULL)
	freeSudoku(s);
      freeTrail(t);
      freeMarks(m);
      shr->waitThreads--;
      pthread_mutex_unlock(&shr->mtx);
      break;
    } else {
      //printf("Grabbed job.\n");
    }
    job = &shr->jobs[shr->numJobs-- - 1];
    shr->waitThreads--;
    pthread_mutex_unlock(&shr->mtx);
    // Step 2: Extract from job
    if (s != NULL)
      freeSudoku(s);
    s = job->s;
    freeTrail(t);
    freeMarks(m);
    t = makeTrail();
    m = createMarks();
    // Step 3: Solve job
    while (1) {
      int scanEr, restEr;
      int guessID = findGuessCell(s);
      int guess = findGuess(s->cs[guessID], s->sz);
      makeGuess(m, t, s, guessID, guess);
      job->ngs++;

      scanEr = scanSudoku(s, t);
      if (scanEr == -1) {
	restEr = chainRestore(m, t, s, 1);
	if (restEr == -1) {
	  break;
	}
      } else if (isSolved(s)) {
	// TODO: Concurrency add solution to shared info
	// - Obtain lock
	// - Increase num solutions
	// - Copy solution to Solution stack
	// - Unlock
	pthread_mutex_lock(&shr->mtx);
	if (shr->solutions->numSols == shr->solutions->maxSols) {
	  reallocSStack(shr->solutions);
	}
	shr->solutions->solutions[shr->solutions->numSols] = copySudoku(s);
	
	shr->solutions->numSols++;
	pthread_mutex_unlock(&shr->mtx);
        
	//printf("Solutions: %d\n", ++sol);
	restEr = chainRestore(m, t, s, 1);
	if (restEr == -1) {
	  break;
	}
      } 
    }
    //printf("Completed job %d!\n", ++jobs);
  }
}

void* solveSudokuThreads(Sudoku* s, int nt) {
  Sudoku* copy = copySudoku(s);
  SharedInfo shr;
  shr.stillBranching = 1;
  shr.numJobs = 0;
  shr.maxJobs = 100;
  Job* jobs = (Job*)malloc(sizeof(Job) * shr.maxJobs);
  shr.jobs = jobs;
  shr.waitThreads = 0;
  shr.numThreads = nt;
  shr.solutions = makeSStack();
  pthread_mutex_init(&shr.mtx, NULL);
  pthread_cond_init(&shr.done, NULL);

  TBInfo tb;
  tb.maxDepth = 1;
  tb.s = copy;
  tb.t = makeTrail();
  tb.m = createMarks();
  tb.SI = &shr;

  pthread_create(&tb.name, NULL, trailBlaze, &tb);

  ThreadInfo ti[nt];
  for (int i = 0; i < nt; i++) {
    ti[i].job = NULL;
    ti[i].t = makeTrail();
    ti[i].m = createMarks();
    ti[i].SI = &shr;
  }
  for (int i = 0; i < nt; i++) {
    pthread_create(&ti[i].name, NULL, solveThread, &ti[i]);
  }
  pthread_join(tb.name, NULL);
  for (int i = 0; i < nt; i++) {
    pthread_join(ti[i].name, NULL);
  }
  pthread_mutex_destroy(&shr.mtx);
  pthread_cond_destroy(&shr.done);
  printf("Success! There are %d solutions.\n", shr.solutions->numSols);
  printf("View solutions? (yes/no)\n");
  char buffer[128];
  char ch;
  int i = 0;
  while (i < sizeof(buffer) && (ch = getchar()) != '\n' && ch != EOF)
    buffer[i++] = ch;
  buffer[i] = 0;
  if (strcmp("yes", buffer) == 0) {
    for (int i = 0; i < shr.solutions->numSols; i++) {
      printSudoku(shr.solutions->solutions[i]);
    } 
  }
  freeSStack(shr.solutions);
  free(jobs);
  freeSudoku(copy);
}

// Main (for testing purposes only)

void printCommands() {
  printf("help - print a list of commands\n");
  printf("quit - quit the program\n");
  printf("import - import a sudoku");
}

int main(int argc, char* argv[]) {
  char buffer[128];
  int running = 1;
  Sudoku* s = NULL;
  while (running) {
    // Receive command
    printf("> "); fflush(stdout);
    int i = 0;
    char ch;
    while (i < sizeof(buffer) && (ch = getchar()) != '\n' && ch != EOF)
      buffer[i++] = ch;
    buffer[i] = 0;
    if (strcmp("q", buffer) == 0 || strcmp("quit", buffer) == 0) {
      if (s != NULL)
	freeSudoku(s);
      running = 0;
    } else if (strcmp("h", buffer) == 0 || strcmp("help", buffer) == 0) {
      printCommands();
    } else if (strcmp("i", buffer) == 0 || strcmp("import", buffer) == 0) {
      int sz;
      printf("What size sudoku are you importing?\n");
      int er = scanf("%d", &sz);
      while(ch = getchar() != '\n'){}
      if (er < 1 || sz < 4 || sz > 81 || sqrt(sz) * sqrt(sz) != sz) {
	printf("Invalid size. Aborting import.\n");
	continue;
      }
      printf("What file would you like to import?\n");
      i = 0;
      while (i < sizeof(buffer) && (ch = getchar()) != '\n' && ch != EOF)
	buffer[i++] = ch;
      buffer[i] = 0;
      s = importSudoku(buffer, sz);
      if (s == NULL) {
	printf("File name invalid. Aborting import..\n");
      }
    } else if (strcmp("r", buffer) == 0 || strcmp("run", buffer) == 0) {
      if (s == NULL) {
	printf("No sudoku available. Please import/make a sudoku first.\n");
	continue;
      }
      printf("Use threads? (yes/no)\n");
      i = 0;
      while (i < sizeof(buffer) && (ch = getchar()) != '\n' && ch != EOF)
	buffer[i++] = ch;
      buffer[i] = 0;
      if (strcmp("yes", buffer) == 0) {
	printf("How many?\n");
	int nt;
	int er = scanf("%d", &nt);
	while(ch = getchar() != '\n'){}
	if (er < 1 || nt < 1) {
	  printf("Invalid size. Aborting import.\n");
	  continue;
	}
	solveSudokuThreads(s, 4);
      } else if (strcmp("no", buffer) == 0) {
	solveSudoku(s);
      } else {
	printf("Not a yes/no. Aborting.\n");
      }
    } else {
      printf("Invalid command. Please try again.\n");
    }
  }
}

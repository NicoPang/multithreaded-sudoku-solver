CC = gcc
CFLAGS = -std=c99 -lm

%.o: %.c
	$(CC) -g -c $(CFLAGS) $<

solver: cells.o trail.o sudoku.o solver.o
	$(CC) -g $(CFLAGS) -o $@ $^ -pthread

clean:
	rm -rf *~ *.o cells trail sudoku solver

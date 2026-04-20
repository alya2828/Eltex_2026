CC = gcc
CFLAGS = -Wall -Wextra -std=c11 

TARGETS = prog

all: $(TARGETS)

prog: prog.c
	$(CC) $(CFLAGS) -pthread -o prog prog.c

clean:
	rm -f $(TARGETS) *.o
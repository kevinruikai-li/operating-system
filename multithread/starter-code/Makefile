# Makefile
CC = gcc
CFLAGS = -pthread -std=c99 -Wall -O2
FMT=indent

all: main test_queue

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

main.o: main.c queue.h
	$(CC) $(CFLAGS) -c main.c

test_queue.o: test_queue.c queue.h
	$(CC) $(CFLAGS) -c test_queue.c

main: queue.o main.o
	$(CC) $(CFLAGS) queue.o main.o -o main

test_queue: queue.o test_queue.o
	$(CC) $(CFLAGS) queue.o test_queue.o -o test_queue

style: main.c queue.c queue.h test_queue.c
	$(FMT) $?

clean:
	$(RM) *.o main test_queue *~

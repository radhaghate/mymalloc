CC = gcc
CFLAGS = -Wall -Werror -g

all: memgrind test1 test2 test3

memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o memgrind memgrind.o mymalloc.o

test1: test1.o mymalloc.o
	$(CC) $(CFLAGS) -o test1 test1.o mymalloc.o

test2: test2.o mymalloc.o
	$(CC) $(CFLAGS) -o test2 test2.o mymalloc.o

test3: test3.o mymalloc.o
	$(CC) $(CFLAGS) -o test3 test3.o mymalloc.o

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c

memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c

test1.o: test1.c mymalloc.h
	$(CC) $(CFLAGS) -c test1.c

test2.o: test2.c mymalloc.h
	$(CC) $(CFLAGS) -c test2.c

test3.o: test3.c mymalloc.h
	$(CC) $(CFLAGS) -c test3.c

clean:
	rm -f *.o memgrind test1 test2 test3
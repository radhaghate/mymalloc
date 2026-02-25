CC = gcc
CFLAGS = -Wall -Wextra -g

TESTS = test1 test2 test3 test4 test5 test6 test7 test8

all: memgrind memtest $(TESTS)

memgrind: memgrind.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o memgrind memgrind.c mymalloc.c

memtest: memtest.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o memtest memtest.c mymalloc.c

test1: test1.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test1 test1.c mymalloc.c

test2: test2.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test2 test2.c mymalloc.c

test3: test3.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test3 test3.c mymalloc.c

test4: test4.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test4 test4.c mymalloc.c

test5: test5.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test5 test5.c mymalloc.c

test6: test6.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test6 test6.c mymalloc.c

test7: test7.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test7 test7.c mymalloc.c

test8: test8.c mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -o test8 test8.c mymalloc.c

clean:
	rm -f memgrind memtest $(TESTS)

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

SRCDIR = src
TESTDIR = tests
INCDIR = include

MYMALLOC_SRC = $(SRCDIR)/mymalloc.c
MYMALLOC_HDR = $(INCDIR)/mymalloc.h

all: memtest memgrind test1 test2 test3 test4 test5 test6 test7 test8

# memtest
memtest: memtest.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o memtest memtest.c $(MYMALLOC_SRC)

# memtest with leak detection enabled
memtest-leak: memtest.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -DLEAK -o memtest-leak memtest.c $(MYMALLOC_SRC)

# memtest using real malloc to verify the test itself is valid
memtest-real: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -o memtest-real memtest.c

# memgrind stress/timing tests
memgrind: memgrind.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o memgrind memgrind.c $(MYMALLOC_SRC)

# correctness tests
test1: $(TESTDIR)/test1.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test1 $(TESTDIR)/test1.c $(MYMALLOC_SRC)

test2: $(TESTDIR)/test2.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test2 $(TESTDIR)/test2.c $(MYMALLOC_SRC)

test3: $(TESTDIR)/test3.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test3 $(TESTDIR)/test3.c $(MYMALLOC_SRC)

test4: $(TESTDIR)/test4.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test4 $(TESTDIR)/test4.c $(MYMALLOC_SRC)

test5: $(TESTDIR)/test5.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test5 $(TESTDIR)/test5.c $(MYMALLOC_SRC)

test6: $(TESTDIR)/test6.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test6 $(TESTDIR)/test6.c $(MYMALLOC_SRC)

test7: $(TESTDIR)/test7.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test7 $(TESTDIR)/test7.c $(MYMALLOC_SRC)

test8: $(TESTDIR)/test8.c $(MYMALLOC_SRC) $(MYMALLOC_HDR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test8 $(TESTDIR)/test8.c $(MYMALLOC_SRC)

# run all non-error tests
test: all
	@echo "Running correctness tests..."
	@echo ""
	@echo "=== memtest ==="
	./memtest
	@echo ""
	@echo "=== memtest-leak (check stderr) ==="
	./memtest-leak
	@echo ""
	@echo "=== test1: allocation and data integrity ==="
	./test1
	@echo ""
	@echo "=== test2: free releases memory ==="
	./test2
	@echo ""
	@echo "=== test3: coalescing ==="
	./test3
	@echo ""
	@echo "=== test4: edge cases ==="
	./test4
	@echo ""
	@echo "=== test8: leak detection (check stderr) ==="
	./test8
	@echo ""
	@echo "=== memgrind ==="
	./memgrind
	@echo ""
	@echo "Tests 5, 6, and 7 exit with code 2 -- run 'make test-errors' to see them."

# run error-detection tests separately since they exit with code 2
test-errors: test5 test6 test7
	@echo "=== test5: free() on a stack variable ==="
	-./test5
	@echo ""
	@echo "=== test6: free() on a mid-chunk pointer ==="
	-./test6
	@echo ""
	@echo "=== test7: double free ==="
	-./test7

clean:
	rm -f memtest memtest-leak memtest-real memgrind \
	      test1 test2 test3 test4 test5 test6 test7 test8 *.o

.PHONY: all test test-errors clean
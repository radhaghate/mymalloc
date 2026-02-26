#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

// test5.c error detection: free() on a stack variable

/* Passes the address of a local int to free().
    myfree() should print an error to stderr and exit with code 2, never reaching last line */

int main() {
    printf("Test 5: free() on a stack variable\n");
    printf("  Should print error to stderr and exit with code 2...\n");
    fflush(stdout);

    int x = 42;
    free(&x);

    // should never reach here
    printf("  ERROR: should have exited already\n");
    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

// test8.c -- leak detection

/* Allocates several objects and intentionally never frees them.
    When the process exits the atexit() leak detector should fire and print something like:
   mymalloc: NNN bytes leaked in 3 objects
*/

int main() {
    printf("Test 8: Leak detection\n");
    printf("  Allocating 3 objects without freeing them...\n");

    void *a = malloc(100);
    void *b = malloc(200);
    void *c = malloc(50);

    if (!a || !b || !c) {
        printf("  ERROR: malloc returned NULL\n");
        return 1;
    }

    printf("  Allocated: 100 bytes at %p\n", a);
    printf("  Allocated: 200 bytes at %p\n", b);
    printf("  Allocated:  50 bytes at %p\n", c);
    printf("  Not freeing any of them.\n");
    printf("  Exiting -- leak detector should report 3 objects to stderr.\n");

    // intentionally not freeing a, b, c
    (void)a; (void)b; (void)c;
    return 0;
}
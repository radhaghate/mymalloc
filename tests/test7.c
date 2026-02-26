#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

// test7.c -- error detection: double free

/* Frees the same chunk twice using an alias pointer (q = p).
 The second free() should detect that the chunk is already free,
 print an error to stderr, and exit with code 2 */

int main() {
    printf("Test 7: Double free\n");

    int *p = malloc(sizeof(int) * 10);
    int *q = p;  // alias

    if (p == NULL) {
        printf("  ERROR: malloc returned NULL\n");
        return 1;
    }

    printf("  Allocated chunk at %p\n", (void *)p);
    printf("  First free (should work fine)...\n");
    fflush(stdout);
    free(p);
    printf("  First free done\n");

    printf("  Second free via alias -- should error and exit with code 2...\n");
    fflush(stdout);
    free(q);

    // should never reach here
    printf("  ERROR: should have exited already\n");
    return 1;
}
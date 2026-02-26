#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

// test6.c -- error detection: free() on a pointer not at the start of a chunk

/* malloc returns a pointer to the start of the payload.
    Calling free(p + 1) is wrong so the allocator should catch it,
    print an error to stderr, and exit with code 2 */

int main() {
    printf("Test 6: free() on a mid-chunk pointer\n");

    int *p = malloc(sizeof(int) * 4);
    if (p == NULL) {
        printf("  ERROR: malloc returned NULL\n");
        return 1;
    }

    printf("  Allocated int array at %p\n", (void *)p);
    printf("  Calling free(p + 1) -- should error and exit with code 2...\n");
    fflush(stdout);

    free(p + 1);

    // should never reach here
    printf("  ERROR: should have exited already\n");
    return 1;
}
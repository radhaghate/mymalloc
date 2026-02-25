#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main() {
    printf("Test 3: Coalescing adjacent free blocks\n");

    // -- part 1: free in order, then request a chunk bigger than any single one
    printf("  Testing forward coalesce (free left to right)...\n");

    char *a = malloc(48);
    char *b = malloc(48);
    char *c = malloc(48);

    if (!a || !b || !c) {
        printf("  ERROR: initial allocs failed\n");
        return 1;
    }

    free(a);
    free(b);
    free(c);

    // 3 chunks of 48 bytes freed -- we should be able to get 120+ bytes now
    char *big = malloc(120);
    if (big == NULL) {
        printf("  ERROR: large alloc after forward coalesce failed\n");
        return 1;
    }
    printf("  Forward coalesce OK\n");
    free(big);

    // -- part 2: free in reverse order
    printf("  Testing reverse coalesce (free right to left)...\n");

    char *x = malloc(48);
    char *y = malloc(48);
    char *z = malloc(48);

    if (!x || !y || !z) {
        printf("  ERROR: initial allocs failed\n");
        return 1;
    }

    free(z);
    free(y);
    free(x);

    big = malloc(120);
    if (big == NULL) {
        printf("  ERROR: large alloc after reverse coalesce failed\n");
        return 1;
    }
    printf("  Reverse coalesce OK\n");
    free(big);

    // -- part 3: free the middle chunk first, then the outer ones
    printf("  Testing coalesce with middle freed first...\n");

    char *p1 = malloc(100);
    char *p2 = malloc(100);
    char *p3 = malloc(100);

    if (!p1 || !p2 || !p3) {
        printf("  ERROR: initial allocs failed\n");
        return 1;
    }

    free(p2);  // middle first
    free(p1);  // then left
    free(p3);  // then right

    big = malloc(280);
    if (big == NULL) {
        printf("  ERROR: large alloc after middle-first coalesce failed\n");
        return 1;
    }
    printf("  Middle-first coalesce OK\n");
    free(big);

    // -- part 4: alternating free (odd indices then even)
    printf("  Testing alternating free pattern...\n");

    char *ptrs[8];
    for (int i = 0; i < 8; i++) {
        ptrs[i] = malloc(32);
        if (ptrs[i] == NULL) {
            printf("  ERROR: malloc failed at %d\n", i);
            return 1;
        }
    }

    for (int i = 1; i < 8; i += 2) free(ptrs[i]);
    for (int i = 0; i < 8; i += 2) free(ptrs[i]);

    // all 8 chunks freed -- should coalesce into one big block
    big = malloc(200);
    if (big == NULL) {
        printf("  ERROR: large alloc after alternating free failed\n");
        return 1;
    }
    printf("  Alternating free coalesce OK\n");
    free(big);

    printf("Test 3 passed!\n");
    return 0;
}
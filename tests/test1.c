#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mymalloc.h"

int main() {
    printf("Test 1: Basic allocation and data integrity\n");

    // allocate objects of several different sizes
    printf("  Allocating objects of different sizes...\n");
    char *p8  = malloc(8);
    char *p16 = malloc(16);
    char *p32 = malloc(32);
    char *p64 = malloc(64);

    if (!p8 || !p16 || !p32 || !p64) {
        printf("  ERROR: one or more mallocs returned NULL\n");
        return 1;
    }
    printf("  Allocations OK\n");

    // every pointer must be 8-byte aligned per the spec
    printf("  Checking alignment...\n");
    int align_errors = 0;
    if ((uintptr_t)p8  % 8 != 0) { printf("  ERROR: p8 not aligned\n");  align_errors++; }
    if ((uintptr_t)p16 % 8 != 0) { printf("  ERROR: p16 not aligned\n"); align_errors++; }
    if ((uintptr_t)p32 % 8 != 0) { printf("  ERROR: p32 not aligned\n"); align_errors++; }
    if ((uintptr_t)p64 % 8 != 0) { printf("  ERROR: p64 not aligned\n"); align_errors++; }
    if (align_errors == 0)
        printf("  All pointers 8-byte aligned\n");

    // write a distinct pattern to each object then check that
    // none of them got overwritten by a neighboring allocation
    printf("  Writing byte patterns...\n");
    memset(p8,  0xAA, 8);
    memset(p16, 0xBB, 16);
    memset(p32, 0xCC, 32);
    memset(p64, 0xDD, 64);

    printf("  Checking data integrity...\n");
    int errors = 0;
    for (int i = 0; i < 8;  i++) if ((unsigned char)p8[i]  != 0xAA) errors++;
    for (int i = 0; i < 16; i++) if ((unsigned char)p16[i] != 0xBB) errors++;
    for (int i = 0; i < 32; i++) if ((unsigned char)p32[i] != 0xCC) errors++;
    for (int i = 0; i < 64; i++) if ((unsigned char)p64[i] != 0xDD) errors++;

    if (errors == 0)
        printf("  Data integrity OK\n");
    else
        printf("  ERROR: %d corrupted bytes\n", errors);

    // free everything and make sure we can allocate again after
    printf("  Freeing objects...\n");
    free(p8);
    free(p16);
    free(p32);
    free(p64);
    printf("  Free OK\n");

    printf("  Testing alloc after free...\n");
    char *again = malloc(32);
    if (again == NULL) {
        printf("  ERROR: malloc after free returned NULL\n");
        return 1;
    }
    free(again);
    printf("  Alloc after free OK\n");

    // malloc(0) should return NULL without crashing
    printf("  Testing malloc(0)...\n");
    char *pzero = malloc(0);
    if (pzero != NULL) {
        printf("  WARNING: malloc(0) returned non-NULL\n");
        free(pzero);
    } else {
        printf("  malloc(0) returned NULL\n");
    }

    // free(NULL) is a no-op in standard C
    printf("  Testing free(NULL)...\n");
    free(NULL);
    printf("  free(NULL) OK\n");

    if (errors == 0 && align_errors == 0)
        printf("Test 1 passed!\n");
    else
        printf("Test 1 FAILED\n");

    return (errors == 0 && align_errors == 0) ? 0 : 1;
}
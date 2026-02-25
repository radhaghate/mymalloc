#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

int main() {
    printf("Test 2: Free actually releases memory\n");

    // fill the whole heap with 24-byte objects (24 + 8 header = 32 bytes each)
    // 4096 / 32 = 128 chunks
    printf("  Filling heap with 128 objects...\n");

    char *ptrs[128];
    int filled = 0;

    for (int i = 0; i < 128; i++) {
        ptrs[i] = malloc(24);
        if (ptrs[i] == NULL) {
            printf("  Heap full after %d objects\n", i);
            break;
        }
        filled++;
    }

    if (filled == 0) {
        printf("  ERROR: couldn't allocate anything\n");
        return 1;
    }
    printf("  Filled heap with %d objects\n", filled);

    // heap should be full -- next malloc should fail
    printf("  Verifying heap is full...\n");
    char *extra = malloc(8);
    if (extra != NULL) {
        printf("  WARNING: malloc succeeded when heap should be full\n");
        free(extra);
    } else {
        printf("  Heap correctly reports full\n");
    }

    // free everything
    printf("  Freeing all objects...\n");
    for (int i = 0; i < filled; i++)
        free(ptrs[i]);
    printf("  All freed\n");

    // fill the heap a second time -- must work if free() did its job
    printf("  Re-filling heap...\n");
    int refilled = 0;
    for (int i = 0; i < filled; i++) {
        ptrs[i] = malloc(24);
        if (ptrs[i] == NULL) {
            printf("  ERROR: re-fill failed at object %d -- free() didn't release memory\n", i);
            return 1;
        }
        memset(ptrs[i], i & 0xFF, 24);
        refilled++;
    }
    printf("  Re-filled %d objects\n", refilled);

    // data integrity check on the second fill
    printf("  Checking data integrity of second fill...\n");
    int corrupt = 0;
    for (int i = 0; i < refilled; i++) {
        for (int j = 0; j < 24; j++) {
            if ((unsigned char)ptrs[i][j] != (unsigned char)(i & 0xFF))
                corrupt++;
        }
    }
    if (corrupt == 0)
        printf("  No data corruption\n");
    else
        printf("  ERROR: %d corrupted bytes\n", corrupt);

    for (int i = 0; i < refilled; i++)
        free(ptrs[i]);

    if (corrupt == 0)
        printf("Test 2 passed!\n");
    else
        printf("Test 2 FAILED\n");

    return corrupt == 0 ? 0 : 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mymalloc.h"

int main() {
    printf("Test 4: Edge cases\n");

    // request way more than the heap can hold
    printf("  Testing over-size allocation...\n");
    void *huge = malloc(10000);
    if (huge != NULL) {
        printf("  ERROR: malloc(10000) should have returned NULL\n");
        free(huge);
        return 1;
    }
    printf("  malloc(10000) correctly returned NULL\n");

    // request exactly as much as the heap holds (minus the header)
    // -- should fail because the header itself takes 8 bytes
    printf("  Testing malloc(4096)...\n");
    void *full = malloc(4096);
    if (full != NULL) {
        printf("  ERROR: malloc(4096) should have returned NULL (no room for header)\n");
        free(full);
        return 1;
    }
    printf("  malloc(4096) correctly returned NULL\n");

    // sizes that are not multiples of 8 -- allocator must round them up
    printf("  Testing unaligned request sizes...\n");
    void *p1 = malloc(1);
    void *p9 = malloc(9);
    void *p17 = malloc(17);
    if (!p1 || !p9 || !p17) {
        printf("  ERROR: malloc failed for unaligned size\n");
        return 1;
    }
    // write to the full rounded-up region to catch any underallocation
    memset(p1,  0x11, 1);
    memset(p9,  0x22, 9);
    memset(p17, 0x33, 17);
    printf("  Unaligned sizes handled OK\n");
    free(p1);
    free(p9);
    free(p17);

    // alignment check on a range of sizes 1..16
    printf("  Checking alignment for sizes 1 through 16...\n");
    void *ptrs[16];
    int align_errors = 0;
    for (int i = 0; i < 16; i++) {
        ptrs[i] = malloc(i + 1);
        if (ptrs[i] == NULL) {
            printf("  ERROR: malloc(%d) returned NULL\n", i + 1);
            return 1;
        }
        if ((uintptr_t)ptrs[i] % 8 != 0) {
            printf("  ERROR: malloc(%d) returned unaligned pointer\n", i + 1);
            align_errors++;
        }
    }
    for (int i = 0; i < 16; i++) free(ptrs[i]);
    if (align_errors == 0)
        printf("  All pointers aligned\n");
    else
        printf("  ERROR: %d alignment errors\n", align_errors);

    // repeated alloc/free cycles -- heap must stay consistent
    printf("  Testing repeated alloc/free cycles...\n");
    for (int cycle = 0; cycle < 20; cycle++) {
        void *p = malloc(100);
        if (p == NULL) {
            printf("  ERROR: malloc failed on cycle %d\n", cycle);
            return 1;
        }
        memset(p, cycle & 0xFF, 100);
        int bad = 0;
        for (int i = 0; i < 100; i++)
            if ((unsigned char)((char *)p)[i] != (unsigned char)(cycle & 0xFF))
                bad++;
        if (bad) {
            printf("  ERROR: data corruption on cycle %d\n", cycle);
            return 1;
        }
        free(p);
    }
    printf("  All cycles OK\n");

    if (align_errors == 0)
        printf("Test 4 passed!\n");
    else
        printf("Test 4 FAILED\n");

    return align_errors == 0 ? 0 : 1;
}
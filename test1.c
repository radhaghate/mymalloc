#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

int main(void) {
    char *ptrs[8];
    int errors = 0;
    for (int i = 0; i < 8; i++) ptrs[i] = malloc(32);
    for (int i = 0; i < 8; i++) memset(ptrs[i], i, 32);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 32; j++)
            if ((unsigned char)ptrs[i][j] != (unsigned char)i) errors++;
    for (int i = 0; i < 8; i++) free(ptrs[i]);
    if (errors == 0)
        printf("PASS: no overlap between allocations\n");
    else
        printf("FAIL: %d corrupted bytes detected\n", errors);
    return errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

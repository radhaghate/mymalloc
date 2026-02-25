#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    char *ptrs[10];
    for (int i = 0; i < 10; i++) ptrs[i] = malloc(8);
    for (int i = 0; i < 10; i++) free(ptrs[i]);
    char *big = malloc(80);
    if (big != NULL) {
        printf("PASS: adjacent free blocks coalesced\n");
        free(big);
        return EXIT_SUCCESS;
    }
    printf("FAIL: could not allocate after coalescing\n");
    return EXIT_FAILURE;
}

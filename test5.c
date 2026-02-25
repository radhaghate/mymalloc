#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    char *p = malloc(0);
    if (p == NULL) {
        printf("PASS: malloc(0) returned NULL\n");
        return EXIT_SUCCESS;
    }
    printf("FAIL: malloc(0) should return NULL\n");
    free(p);
    return EXIT_FAILURE;
}

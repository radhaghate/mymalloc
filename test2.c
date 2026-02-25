#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    char *p = malloc(64);
    free(p);
    char *q = malloc(64);
    if (q != NULL) {
        printf("PASS: freed memory was reused\n");
        free(q);
        return EXIT_SUCCESS;
    }
    printf("FAIL: malloc returned NULL after free\n");
    return EXIT_FAILURE;
}

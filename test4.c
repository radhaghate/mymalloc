#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    char *p = malloc(4096);
    if (p == NULL) {
        printf("PASS: malloc returned NULL when out of memory\n");
        return EXIT_SUCCESS;
    }
    printf("FAIL: malloc should have returned NULL\n");
    free(p);
    return EXIT_FAILURE;
}

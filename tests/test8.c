#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    int *p = malloc(sizeof(int));
    int *q = p;
    free(p);
    free(q);
    return EXIT_SUCCESS;
}

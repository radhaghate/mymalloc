#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    int *p = malloc(sizeof(int) * 2);
    free(p + 1);
    return EXIT_SUCCESS;
}

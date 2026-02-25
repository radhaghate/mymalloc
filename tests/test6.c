#include <stdlib.h>
#include "mymalloc.h"

int main(void) {
    int x = 5;
    free(&x);
    return EXIT_SUCCESS;
}

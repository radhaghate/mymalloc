#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main() {
    int *p = malloc(sizeof(int) * 100);
    int *q = p;
    
    free(p);
    free(q); 
    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main() {
    int x;
    free(&x); 
    
    return 0;
}
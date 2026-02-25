#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main() {
    char *obj1 = malloc(100);
    char *obj2 = malloc(200);

    for (int i = 0; i < 100; i++) obj1[i] = 1;
    for (int i = 0; i < 200; i++) obj2[i] = 2;

    int integrity_passed = 1;
    for (int i = 0; i < 100; i++) {
        if (obj1[i] != 1) integrity_passed = 0;
    }
    for (int i = 0; i < 200; i++) {
        if (obj2[i] != 2) integrity_passed = 0;
    }

    if (integrity_passed) {
        printf("Data integrity test passed.\n");
    }

    free(obj1);
    free(obj2);

    char *obj3 = malloc(300);
    if (obj3 == obj1) {
        printf("Coalescing test passed.\n");
    }

    free(obj3);

    return 0;
}
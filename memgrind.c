#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mymalloc.h"

#define RUNS 50

static double elapsed_ms(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_usec - start.tv_usec) / 1000.0;
}

static void task1(void) {
    for (int i = 0; i < 120; i++) {
        char *p = malloc(1);
        free(p);
    }
}

static void task2(void) {
    char *ptrs[120];
    for (int i = 0; i < 120; i++) ptrs[i] = malloc(1);
    for (int i = 0; i < 120; i++) free(ptrs[i]);
}

static void task3(void) {
    char *ptrs[120];
    int count = 0, total = 0;
    memset(ptrs, 0, sizeof(ptrs));
    while (total < 120) {
        if (count == 0 || (rand() % 2 == 0)) {
            if (count < 120) {
                ptrs[count++] = malloc(1);
                total++;
            }
        } else {
            int idx = rand() % count;
            free(ptrs[idx]);
            ptrs[idx] = ptrs[--count];
        }
    }
    for (int i = 0; i < count; i++) free(ptrs[i]);
}

static void task4(void) {
    char *ptrs[30];
    for (int i = 0; i < 30; i++) ptrs[i] = malloc((i % 8) + 1);
    for (int i = 29; i >= 0; i--) free(ptrs[i]);
}

static void task5(void) {
    char *queue[90];
    int head = 0, tail = 0;
    for (int i = 0; i < 60; i++) {
        queue[tail++] = malloc(4);
    }
    for (int i = 0; i < 30; i++) {
        free(queue[head++]);
        queue[tail++] = malloc(4);
    }
    while (head < tail) free(queue[head++]);
}

static void run_task(const char *name, void (*task)(void)) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < RUNS; i++) task();
    gettimeofday(&end, NULL);
    printf("%s: %.4f ms avg\n", name, elapsed_ms(start, end) / RUNS);
}

int main(void) {
    run_task("Task 1", task1);
    run_task("Task 2", task2);
    run_task("Task 3", task3);
    run_task("Task 4", task4);
    run_task("Task 5", task5);
    return EXIT_SUCCESS;
}

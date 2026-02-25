#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mymalloc.h"

void run_workloads() {
    for (int i = 0; i < 120; i++) {
        void *ptr = malloc(1);
        free(ptr);
    }

    void *ptrs[120];
    for (int i = 0; i < 120; i++) {
        ptrs[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptrs[i]);
    }

    void *random_ptrs[120];
    int alloc_count = 0;
    int current_allocs = 0;

    while (alloc_count < 120) {
        int choice = rand() % 2;
        if (choice == 0 || current_allocs == 0) {
            random_ptrs[current_allocs] = malloc(1);
            current_allocs++;
            alloc_count++;
        } else {
            current_allocs--;
            free(random_ptrs[current_allocs]);
        }
    }
    for (int i = 0; i < current_allocs; i++) {
        free(random_ptrs[i]);
    }
}

int main() {
    struct timeval start, end;
    long total_time = 0;

    for (int i = 0; i < 50; i++) {
        gettimeofday(&start, NULL);
        
        run_workloads();
        
        gettimeofday(&end, NULL);
        
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        total_time += (seconds * 1000000) + microseconds;
    }

    printf("Average workload time: %ld microseconds\n", total_time / 50);

    return 0;
}
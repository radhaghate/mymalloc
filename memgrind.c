#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mymalloc.h"

struct Node {
    int data;
    struct Node* next;
};

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

    struct Node* head = NULL;
    for (int i = 0; i < 100; i++) {
        struct Node* new_node = malloc(sizeof(struct Node));
        if (new_node != NULL) {
            new_node->data = i;
            new_node->next = head;
            head = new_node;
        }
    }
    struct Node* curr = head;
    while (curr != NULL) {
        struct Node* temp = curr;
        curr = curr->next;
        free(temp);
    }

    void *frag_ptrs[100];
    for (int i = 0; i < 100; i++) {
        frag_ptrs[i] = malloc(8);
    }
    for (int i = 0; i < 100; i += 2) {
        if (frag_ptrs[i] != NULL) free(frag_ptrs[i]);
    }
    for (int i = 1; i < 100; i += 2) {
        if (frag_ptrs[i] != NULL) free(frag_ptrs[i]);
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
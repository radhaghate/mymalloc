#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

#define MEMLENGTH 4096

static union {
    char bytes[MEMLENGTH];
    double not_used;
} heap;


#define HEADER_SIZE 8
#define GET_SIZE(h)   (*(size_t *)(h) & ~(size_t)1)
#define GET_ALLOC(h)  (*(size_t *)(h) & (size_t)1)
#define SET_HEADER(h, sz, alloc) (*(size_t *)(h) = (sz) | (alloc))

static int initialized = 0;

static void leak_detect(void) {
    int count = 0;
    size_t total = 0;
    char *p = heap.bytes;
    while (p < heap.bytes + MEMLENGTH) {
        if (GET_ALLOC(p)) {
            count++;
            total += GET_SIZE(p);
        }
        p += HEADER_SIZE + GET_SIZE(p);
    }
    if (count > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", total, count);
    }
}

static void init_heap(void) {
    SET_HEADER(heap.bytes, MEMLENGTH - HEADER_SIZE, 0);
    initialized = 1;
    atexit(leak_detect);
}

void *mymalloc(size_t size, char *file, int line) {
    if (!initialized) init_heap();

    if (size == 0) return NULL;

    size = (size + 7) & ~(size_t)7;

    char *p = heap.bytes;
    while (p < heap.bytes + MEMLENGTH) {
        size_t csz = GET_SIZE(p);
        int calloc = GET_ALLOC(p);
        if (!calloc && csz >= size) {
            if (csz >= size + HEADER_SIZE + 8) {
                char *next = p + HEADER_SIZE + size;
                SET_HEADER(next, csz - size - HEADER_SIZE, 0);
                SET_HEADER(p, size, 1);
            } else {
                SET_HEADER(p, csz, 1);
            }
            return p + HEADER_SIZE;
        }
        p += HEADER_SIZE + csz;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    if (!initialized) init_heap();

    if (ptr == NULL ||
        (char *)ptr < heap.bytes + HEADER_SIZE ||
        (char *)ptr >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    char *p = heap.bytes;
    while (p < heap.bytes + MEMLENGTH) {
        char *payload = p + HEADER_SIZE;
        size_t csz = GET_SIZE(p);
        if (payload == (char *)ptr) {
            if (!GET_ALLOC(p)) {
                fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }
            SET_HEADER(p, csz, 0);

            char *next = p + HEADER_SIZE + GET_SIZE(p);
            while (next < heap.bytes + MEMLENGTH && !GET_ALLOC(next)) {
                SET_HEADER(p, GET_SIZE(p) + HEADER_SIZE + GET_SIZE(next), 0);
                next = p + HEADER_SIZE + GET_SIZE(p);
            }
            return;
        }
        p += HEADER_SIZE + csz;
    }

    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}

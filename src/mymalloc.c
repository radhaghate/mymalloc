#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define HEADER_SIZE 8
#define MIN_CHUNK_SIZE 16  /* header (8) + minimum payload (8) */

static union {
    char bytes[MEMLENGTH];
    double not_used;  /* forces 8-byte alignment on heap.bytes */
} heap;

static int initialized = 0;

/*
 * Header layout: single size_t where the low bit is the alloc flag.
 * All sizes are multiples of 8, so the low 3 bits are always available.
 */
#define GET_SIZE(p)          (*(size_t *)(p) & ~(size_t)1)
#define GET_ALLOC(p)         (*(size_t *)(p) &  (size_t)1)
#define SET_HEADER(p, sz, a) (*(size_t *)(p) = (sz) | (a))

/* runs at exit, reports any still-allocated chunks */
static void leak_detect(void) {
    int count = 0;
    size_t total = 0;
    char *p = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (p < end) {
        size_t sz = GET_SIZE(p);
        if (sz == 0) break;
        if (GET_ALLOC(p)) {
            count++;
            total += sz;
        }
        p += HEADER_SIZE + sz;
    }

    if (count > 0)
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", total, count);
}

static void init_heap(void) {
    SET_HEADER(heap.bytes, MEMLENGTH - HEADER_SIZE, 0);
    initialized = 1;
    atexit(leak_detect);
}

/* first-fit: returns pointer to chunk header, or NULL */
static char *find_free(size_t size) {
    char *p = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (p < end) {
        size_t sz = GET_SIZE(p);
        if (sz == 0) break;
        if (!GET_ALLOC(p) && sz >= size)
            return p;
        p += HEADER_SIZE + sz;
    }

    return NULL;
}

/* split chunk at p into size + leftover, if leftover is big enough to use */
static void split_chunk(char *p, size_t size) {
    size_t sz = GET_SIZE(p);
    if (sz >= size + MIN_CHUNK_SIZE) {
        char *next = p + HEADER_SIZE + size;
        SET_HEADER(next, sz - size - HEADER_SIZE, 0);
        SET_HEADER(p, size, 0);
    }
}

/* merge adjacent free chunks, called after every free */
static void coalesce(void) {
    char *p = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (p < end) {
        size_t sz = GET_SIZE(p);
        if (sz == 0) break;

        if (!GET_ALLOC(p)) {
            char *next = p + HEADER_SIZE + sz;
            while (next < end && GET_SIZE(next) > 0 && !GET_ALLOC(next)) {
                sz = sz + HEADER_SIZE + GET_SIZE(next);
                SET_HEADER(p, sz, 0);
                next = p + HEADER_SIZE + sz;
            }
        }

        p += HEADER_SIZE + GET_SIZE(p);
    }
}

/* basic bounds + alignment check before doing the full heap walk */
static int in_heap(void *ptr) {
    char *p = (char *)ptr;
    if (p < heap.bytes + HEADER_SIZE || p >= heap.bytes + MEMLENGTH)
        return 0;
    if ((uintptr_t)p % 8 != 0)
        return 0;
    return 1;
}

void *mymalloc(size_t size, char *file, int line) {
    if (!initialized) init_heap();

    if (size == 0) return NULL;

    /* keep original for the error message, align the working copy */
    size_t orig = size;
    size = (size + 7) & ~(size_t)7;

    char *p = find_free(size);
    if (p == NULL) {
        coalesce();
        p = find_free(size);
    }

    if (p == NULL) {
        fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n",
                orig, file, line);
        return NULL;
    }

    split_chunk(p, size);
    SET_HEADER(p, GET_SIZE(p), 1);
    return p + HEADER_SIZE;
}

void myfree(void *ptr, char *file, int line) {
    if (!initialized) init_heap();

    if (ptr == NULL) return;

    if (!in_heap(ptr)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    /* walk the chunk list to confirm ptr is actually a payload start */
    char *p = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (p < end) {
        size_t sz = GET_SIZE(p);
        if (sz == 0) break;

        if (p + HEADER_SIZE == (char *)ptr) {
            if (!GET_ALLOC(p)) {
                /* already free — double free */
                fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }
            SET_HEADER(p, sz, 0);
            coalesce();
            return;
        }

        p += HEADER_SIZE + sz;
    }

    /* got through the whole heap without finding a matching chunk start */
    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}
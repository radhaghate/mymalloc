#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "mymalloc.h"

/* ─────────────────────────────────────────────
 * Heap storage
 * The union forces 8-byte alignment on heap.bytes.
 * ───────────────────────────────────────────── */
#define MEMLENGTH 4096

static union {
    char   bytes[MEMLENGTH];
    double not_used;          /* never used; just enforces alignment */
} heap;

/* ─────────────────────────────────────────────
 * Chunk header  (exactly 8 bytes)
 *
 * We pack size and the free-flag into one 8-byte
 * value.  Because every size is a multiple of 8,
 * the low 3 bits of size are always 0.  We steal
 * bit 0 as the "is_free" flag:
 *
 *   stored_value = payload_size | is_free
 *
 * Macros below read/write the two fields cleanly.
 * ───────────────────────────────────────────── */
typedef struct {
    size_t info;   /* high bits = payload size, bit 0 = free flag */
} Header;

#define HEADERSIZE  sizeof(Header)          /* 8 bytes                */
#define GET_SIZE(h)    ((h)->info & ~(size_t)7)   /* mask off low bits */
#define GET_FREE(h)    ((h)->info & 1)
#define SET(h, sz, fr) ((h)->info = ((sz) | (fr)))

/* smallest payload we can store (must be >= 8 and a multiple of 8) */
#define MIN_PAYLOAD 8

/* ─────────────────────────────────────────────
 * Initialisation flag (the ONE static int allowed
 * outside the heap array, per the spec §1.4)
 * ───────────────────────────────────────────── */
static int initialized = 0;

/* ─────────────────────────────────────────────
 * Forward declaration of leak detector
 * ───────────────────────────────────────────── */
static void leak_detector(void);

/* ─────────────────────────────────────────────
 * init()  –  called once before any allocation.
 * Sets up the entire heap as a single free chunk
 * and registers the leak detector with atexit().
 * ───────────────────────────────────────────── */
static void init(void) {
    Header *h = (Header *)heap.bytes;
    SET(h, MEMLENGTH - HEADERSIZE, 1);   /* one big free chunk */
    initialized = 1;
    atexit(leak_detector);
}

/* ─────────────────────────────────────────────
 * round_up(size)
 * Returns the smallest multiple of 8 >= size.
 * ───────────────────────────────────────────── */
static size_t round_up(size_t size) {
    return (size + 7) & ~(size_t)7;
}

/* ─────────────────────────────────────────────
 * mymalloc()
 *
 * First-fit allocator.
 *   1. Round the request up to a multiple of 8.
 *   2. Walk the chunk list looking for a free
 *      chunk that is large enough.
 *   3. Split the chunk if the leftover would
 *      fit a minimal chunk (header + MIN_PAYLOAD).
 *   4. Return a pointer to the payload.
 * ───────────────────────────────────────────── */
void *mymalloc(size_t size, char *file, int line) {
    if (!initialized) init();

    if (size == 0) {
        fprintf(stderr, "malloc: Unable to allocate 0 bytes (%s:%d)\n",
                file, line);
        return NULL;
    }

    size_t needed = round_up(size);   /* payload bytes, rounded up */

    char *cur = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (cur < end) {
        Header *h = (Header *)cur;
        size_t  csz = GET_SIZE(h);

        if (GET_FREE(h) && csz >= needed) {
            /* Can we split?  Need room for a new header + at least MIN_PAYLOAD. */
            if (csz >= needed + HEADERSIZE + MIN_PAYLOAD) {
                /* carve out a new free chunk after this one */
                Header *next = (Header *)(cur + HEADERSIZE + needed);
                SET(next, csz - needed - HEADERSIZE, 1);
                SET(h, needed, 0);          /* mark allocated, trimmed size */
            } else {
                SET(h, csz, 0);             /* use whole chunk, mark allocated */
            }
            return (void *)(cur + HEADERSIZE);   /* payload starts here */
        }

        /* advance to next chunk */
        cur += HEADERSIZE + csz;
    }

    /* no suitable chunk found */
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n",
            size, file, line);
    return NULL;
}

/* ─────────────────────────────────────────────
 * myfree()
 *
 * Validates ptr, marks the chunk free, then
 * coalesces it with any immediately following
 * free chunks (forward coalescing).
 *
 * Errors caught:
 *   • pointer outside the heap
 *   • pointer not at any chunk boundary
 *   • pointer to a chunk that is already free
 *     (double-free)
 * ───────────────────────────────────────────── */
void myfree(void *ptr, char *file, int line) {
    if (!initialized) init();

    char *p   = (char *)ptr;
    char *cur = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    /* ── quick range check ── */
    if (p < heap.bytes + HEADERSIZE || p >= end) {
        fprintf(stderr,
                "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    /* ── walk the chunk list to find which chunk ptr belongs to ── */
    while (cur < end) {
        Header *h       = (Header *)cur;
        size_t  csz     = GET_SIZE(h);
        char   *payload = cur + HEADERSIZE;

        if (payload == p) {
            /* found the matching chunk */

            if (GET_FREE(h)) {
                /* already free → double-free */
                fprintf(stderr,
                        "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }

            /* mark this chunk free */
            SET(h, csz, 1);

            /* ── forward coalescing ── */
            char *next = cur + HEADERSIZE + GET_SIZE(h);
            while (next < end) {
                Header *nh = (Header *)next;
                if (!GET_FREE(nh)) break;
                /* merge nh into h */
                SET(h, GET_SIZE(h) + HEADERSIZE + GET_SIZE(nh), 1);
                next += HEADERSIZE + GET_SIZE(nh);
            }

            return;   /* success */
        }

        /* ptr is inside this chunk's payload but not at the start */
        if (p > payload && p < payload + (ptrdiff_t)csz) {
            fprintf(stderr,
                    "free: Inappropriate pointer (%s:%d)\n", file, line);
            exit(2);
        }

        cur += HEADERSIZE + csz;
    }

    /* ptr didn't match any chunk boundary */
    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}

/* ─────────────────────────────────────────────
 * leak_detector()
 *
 * Registered with atexit().  Scans the heap for
 * allocated chunks and reports them to stderr.
 * Must NOT call exit().
 * ───────────────────────────────────────────── */
static void leak_detector(void) {
    int    count = 0;
    size_t total = 0;
    char  *cur   = heap.bytes;
    char  *end   = heap.bytes + MEMLENGTH;

    while (cur < end) {
        Header *h   = (Header *)cur;
        size_t  csz = GET_SIZE(h);

        if (!GET_FREE(h)) {
            count++;
            total += csz;
        }

        cur += HEADERSIZE + csz;
    }

    if (count > 0) {
        fprintf(stderr,
                "mymalloc: %zu bytes leaked in %d object%s.\n",
                total, count, count == 1 ? "" : "s");
    }
}
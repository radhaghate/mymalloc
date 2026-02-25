/*
 * mymalloc.c
 *
 * Custom implementation of malloc() and free() backed by a 4096-byte
 * static array.  Satisfies all requirements from CS 214 Project 1:
 *
 *   §1.1  Coalescing adjacent free chunks in free()
 *   §1.2  8-byte alignment for all payloads; header is exactly 8 bytes
 *   §1.3  First-fit allocation with splitting
 *   §1.4  Lazy initialisation via a single static flag; atexit() leak detector
 *   §2.1  Detects & reports three free() error cases, then exits with status 2
 *   §2.2  Leak detector reports count and total payload bytes at process exit
 *
 * Header layout (8 bytes total):
 *   We store everything in a single size_t.  Because every payload size is
 *   rounded up to a multiple of 8, the lowest three bits of the size are
 *   always 0.  We steal bit 0 as the "is_free" flag:
 *
 *       stored = payload_size | is_free_flag
 *
 *   GET_SIZE masks the low bits off; GET_FREE reads bit 0.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

/* ── heap storage ───────────────────────────────────────────────────────── */

#define MEMLENGTH 4096

static union {
    char   bytes[MEMLENGTH];
    double not_used;        /* forces 8-byte alignment on bytes[] */
} heap;

/* ── header type & accessors ────────────────────────────────────────────── */

typedef struct {
    size_t info;            /* high bits: payload size  |  bit 0: free flag */
} Header;

#define HEADERSIZE      sizeof(Header)          /* exactly 8 bytes          */
#define GET_SIZE(h)     ((h)->info & ~(size_t)7)
#define GET_FREE(h)     ((h)->info & (size_t)1)
#define SET(h, sz, fr)  ((h)->info = ((sz) | (size_t)(fr)))

/* minimum payload we will store (must be a multiple of 8) */
#define MIN_PAYLOAD     8

/* ── initialisation flag (the one allowed static int outside the heap) ─── */

static int initialized = 0;

/* ── forward declarations ───────────────────────────────────────────────── */

static void init(void);
static void leak_detector(void);
static size_t round_up8(size_t n);

/* ── init ───────────────────────────────────────────────────────────────── */
/*
 * Called automatically on the first mymalloc() or myfree() call.
 * Sets up the entire heap as one large free chunk and registers the
 * leak detector to run when the process exits.
 */
static void init(void) {
    Header *h = (Header *)heap.bytes;
    SET(h, MEMLENGTH - HEADERSIZE, 1);  /* one big free chunk */
    initialized = 1;
    atexit(leak_detector);
}

/* ── round_up8 ──────────────────────────────────────────────────────────── */
/*
 * Returns the smallest multiple of 8 that is >= n.
 * Example: round_up8(1) == 8, round_up8(8) == 8, round_up8(9) == 16.
 */
static size_t round_up8(size_t n) {
    return (n + 7) & ~(size_t)7;
}

/* ── mymalloc ───────────────────────────────────────────────────────────── */
/*
 * First-fit allocator.
 *
 *  1. Round the requested size up to the next multiple of 8.
 *  2. Walk every chunk in the heap looking for a free chunk whose
 *     payload is at least that large.
 *  3. If the leftover after carving out the requested bytes is large
 *     enough to hold a header plus MIN_PAYLOAD bytes, split the chunk.
 *  4. Mark the chunk allocated and return a pointer to its payload.
 *
 * Returns NULL (and prints to stderr) if no suitable chunk is found.
 */
void *mymalloc(size_t size, char *file, int line) {
    if (!initialized) init();

    /* Requesting 0 bytes is not useful; treat as an error. */
    if (size == 0) {
        fprintf(stderr,
                "malloc: Unable to allocate 0 bytes (%s:%d)\n",
                file, line);
        return NULL;
    }

    size_t needed = round_up8(size);    /* payload bytes we must provide */

    char *cur = heap.bytes;
    char *end = heap.bytes + MEMLENGTH;

    while (cur < end) {
        Header *h   = (Header *)cur;
        size_t  csz = GET_SIZE(h);      /* payload size of this chunk   */

        if (GET_FREE(h) && csz >= needed) {

            /*
             * Split only when the remainder is large enough to be a
             * valid chunk on its own (header + at least MIN_PAYLOAD).
             */
            if (csz >= needed + HEADERSIZE + MIN_PAYLOAD) {
                /* build the new free chunk immediately after our slice */
                Header *split = (Header *)(cur + HEADERSIZE + needed);
                SET(split, csz - needed - HEADERSIZE, 1);
                SET(h, needed, 0);      /* allocated, trimmed size */
            } else {
                /* use the whole chunk – don't create a sliver */
                SET(h, csz, 0);
            }

            return (void *)(cur + HEADERSIZE);  /* pointer to payload */
        }

        cur += HEADERSIZE + csz;
    }

    /* No chunk is large enough. */
    fprintf(stderr,
            "malloc: Unable to allocate %zu bytes (%s:%d)\n",
            size, file, line);
    return NULL;
}

/* ── myfree ─────────────────────────────────────────────────────────────── */
/*
 * Validates ptr, marks the owning chunk free, then coalesces it with
 * any immediately following free chunks.
 *
 * Detected errors (all print a message and call exit(2)):
 *   1. ptr is outside the heap                        (not from malloc)
 *   2. ptr does not point to the start of any chunk   (bad offset)
 *   3. ptr points to a chunk that is already free     (double-free)
 */
void myfree(void *ptr, char *file, int line) {
    if (!initialized) init();

    char *p   = (char *)ptr;
    char *end = heap.bytes + MEMLENGTH;

    /*
     * Quick range check: a valid payload pointer must lie strictly
     * inside the heap and be at least HEADERSIZE bytes from the start
     * (because every payload is preceded by a header).
     */
    if (p < heap.bytes + HEADERSIZE || p >= end) {
        fprintf(stderr,
                "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    /*
     * Walk the chunk list to find the chunk whose payload starts at p.
     * We also detect the case where p falls inside a chunk's payload
     * (but is not at the start), which is also an error.
     */
    char *cur = heap.bytes;

    while (cur < end) {
        Header *h       = (Header *)cur;
        size_t  csz     = GET_SIZE(h);
        char   *payload = cur + HEADERSIZE;

        if (payload == p) {
            /* Found the matching chunk. */

            /* Error: double-free */
            if (GET_FREE(h)) {
                fprintf(stderr,
                        "free: Inappropriate pointer (%s:%d)\n",
                        file, line);
                exit(2);
            }

            /* Mark this chunk free. */
            SET(h, csz, 1);

            /*
             * Full coalescing pass: after marking free, walk the ENTIRE
             * heap from the beginning and merge any run of consecutive
             * free chunks into one.  This handles both forward and
             * backward neighbours in a single simple pass.
             */
            char *scan = heap.bytes;
            while (scan < end) {
                Header *sh  = (Header *)scan;
                size_t  ssz = GET_SIZE(sh);

                if (GET_FREE(sh)) {
                    /* absorb all following free chunks into sh */
                    char *nxt = scan + HEADERSIZE + GET_SIZE(sh);
                    while (nxt < end) {
                        Header *nh = (Header *)nxt;
                        if (!GET_FREE(nh)) break;
                        SET(sh, GET_SIZE(sh) + HEADERSIZE + GET_SIZE(nh), 1);
                        nxt = scan + HEADERSIZE + GET_SIZE(sh);
                    }
                    /* advance past the now-merged free chunk */
                    scan += HEADERSIZE + GET_SIZE(sh);
                } else {
                    scan += HEADERSIZE + ssz;
                }
            }

            return;  /* success */
        }

        /*
         * p falls inside this chunk's payload but is not at its start –
         * that is an invalid (misaligned) pointer.
         */
        if (p > payload && p < payload + csz) {
            fprintf(stderr,
                    "free: Inappropriate pointer (%s:%d)\n", file, line);
            exit(2);
        }

        cur += HEADERSIZE + csz;
    }

    /*
     * Walked the entire heap without finding a chunk whose payload
     * starts at p.  The pointer did not come from mymalloc().
     */
    fprintf(stderr,
            "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}

/* ── leak_detector ──────────────────────────────────────────────────────── */
/*
 * Registered with atexit() during initialisation.
 * Scans the entire heap and reports any chunks that are still allocated.
 *
 * Per the spec: reports payload bytes only (not header bytes), and must
 * NOT call exit().
 */
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
            total += csz;       /* payload bytes (including alignment padding) */
        }

        cur += HEADERSIZE + csz;
    }

    if (count > 0) {
        fprintf(stderr,
                "mymalloc: %zu bytes leaked in %d object%s.\n",
                total, count, count == 1 ? "" : "s");
    }
}
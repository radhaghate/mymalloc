#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mymalloc.h"

#define MEMLENGTH 4096 

static union {
    char bytes[MEMLENGTH];
    double not_used;
} heap;

typedef struct Chunk {
    size_t size;
    int is_allocated;
} Chunk;

static int initialized = 0; 

void detect_leaks() {
    int leak_count = 0;
    size_t leaked_bytes = 0;
    char *curr = heap.bytes;

    while (curr < heap.bytes + MEMLENGTH) {
        Chunk *header = (Chunk *)curr;
        if (header->is_allocated) {
            leak_count++;
            leaked_bytes += header->size;
        }
        curr += sizeof(Chunk) + header->size; 
    }

    if (leak_count > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", leaked_bytes, leak_count);
    }
}

void initialize_heap() {
    Chunk *first_chunk = (Chunk *)heap.bytes;
    first_chunk->size = MEMLENGTH - sizeof(Chunk);
    first_chunk->is_allocated = 0;

    atexit(detect_leaks); 
    initialized = 1;
}

void *mymalloc(size_t size, char *file, int line) {
    if (!initialized) initialize_heap();

    if (size == 0) return NULL;

    size_t aligned_size = (size + 7) & ~7; 

    char *curr = heap.bytes;

    while (curr < heap.bytes + MEMLENGTH) {
        Chunk *header = (Chunk *)curr;

        if (!header->is_allocated && header->size >= aligned_size) {
            
            if (header->size >= aligned_size + sizeof(Chunk) + 8) {
                Chunk *next_chunk = (Chunk *)(curr + sizeof(Chunk) + aligned_size);
                next_chunk->size = header->size - aligned_size - sizeof(Chunk);
                next_chunk->is_allocated = 0;

                header->size = aligned_size;
            }

            header->is_allocated = 1;
            return curr + sizeof(Chunk);
        }
        curr += sizeof(Chunk) + header->size;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    if (!initialized) initialize_heap();

    if (ptr == NULL || (char *)ptr < heap.bytes || (char *)ptr >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    char *curr = heap.bytes;
    Chunk *prev_chunk = NULL;

    while (curr < heap.bytes + MEMLENGTH) {
        Chunk *header = (Chunk *)curr;
        void *payload = curr + sizeof(Chunk);

        if (payload == ptr) {
            if (!header->is_allocated) {
                fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }

            header->is_allocated = 0;

            char *next = curr + sizeof(Chunk) + header->size;
            if (next < heap.bytes + MEMLENGTH) {
                Chunk *next_header = (Chunk *)next;
                if (!next_header->is_allocated) {
                    header->size += sizeof(Chunk) + next_header->size;
                }
            }

            if (prev_chunk != NULL && !prev_chunk->is_allocated) {
                prev_chunk->size += sizeof(Chunk) + header->size;
            }
            return;
        }

        prev_chunk = header;
        curr += sizeof(Chunk) + header->size;
    }

    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}
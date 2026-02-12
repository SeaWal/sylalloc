#include <stdalign.h>
#include <stdint.h>
#include <sys/mman.h>

#include "sylalloc.h"


// request memory from os using mmap
static void* mmap_alloc(size_t size) {
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if(p == MAP_FAILED) {
        return NULL;
    }

    return p;
}

// TODO: guard against overflow?
static size_t align_up(size_t size) {
    size_t alignment = alignof(max_align_t);
    return (size + alignment - 1) & ~(alignment - 1);
}


void* syl_malloc(size_t size) {
    if(size == 0) { 
        return NULL;
    }

    size_t aligned_size = align_up(size);
    // ensure total_size doesn't overflow
    if(aligned_size > SIZE_MAX - sizeof(memheader_t)) {
        return NULL;
    }

    size_t total_size = sizeof(memheader_t) + aligned_size;

    memheader_t* block = mmap_alloc(total_size);
    if(!block) {
        return NULL;
    }

    block->is_free = false;
    block->size = aligned_size;
    block->next = NULL;

    return (void*)(block + 1);
}
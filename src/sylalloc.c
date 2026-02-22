#include <stdalign.h>
#include <stdint.h>
#include <sys/mman.h>

#include "sylalloc.h"

#define MIN_SPLIT_SIZE 8

// track allocations for re-use
static memheader_t* free_list = NULL;


static inline void* header_to_user_start(memheader_t* block) {
    return (void*)(block + MEMHEADER_SIZE);
}

static inline memheader_t* user_start_to_header(void* ptr) {
    return (memheader_t*)ptr - MEMHEADER_SIZE;
}

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


// try to find a free block using first-fit algo
static memheader_t* find_free_block(size_t size) {
    memheader_t* current = free_list;

    while(current) {
        if(current->size >= size) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}


static void remove_block_from_free_list(memheader_t* block) {
    if(!block || !free_list) {
        return;
    }

    memheader_t* current = free_list;
    memheader_t* prev = NULL;

    while(current) {
        if(current == block) {
            if(prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }
            current->next = NULL;
            current->is_free = false;
            return;
        }

        prev = current;
        current = current->next;
    }
}

static void add_block_to_free_list(memheader_t* block) {
    if(!block) {
        return;
    }

    block->is_free = true;

    // insert block in memory-sorted order

    if(!free_list || block < free_list) {
        block->next = free_list;
        free_list = block;
        return;
    }

    memheader_t* current = free_list;
    while(current->next && current->next < block) {
        current = current->next;
    }

    block->next = current->next;
    current->next = block;
}

static void split_block(memheader_t* block, size_t required_size) {
    size_t extra_size = block->size - required_size;

    // no point splitting if resultant block is too small
    if(extra_size <= MEMHEADER_SIZE + MIN_SPLIT_SIZE) {
        return;
    }

    // create new block just after the current one
    memheader_t* res_block = (memheader_t*)((char*)(block+1) + required_size);
    res_block->size = extra_size - MEMHEADER_SIZE;

    add_block_to_free_list(res_block);

    block->size = required_size;
}


void* syl_malloc(size_t size) {
    if(size == 0) { 
        return NULL;
    }

    size_t aligned_size = align_up(size);
    // ensure total_size doesn't overflow
    if(aligned_size > SIZE_MAX - MEMHEADER_SIZE) {
        return NULL;
    }

    memheader_t* block;

    // re-use existing if possible
    block = find_free_block(aligned_size);

    if(block) {
        remove_block_from_free_list(block);
        split_block(block, aligned_size);
        return header_to_user_start(block);
    }

    size_t total_size = MEMHEADER_SIZE + aligned_size;
    
    block = mmap_alloc(total_size);
    if(!block) {
        return NULL;
    }

    block->is_free = false;
    block->size = aligned_size;
    block->next = NULL;

    return header_to_user_start(block);
}


void syl_free(void* ptr) {
    if(!ptr) {
        return;
    }

    memheader_t* block = user_start_to_header(ptr);
    add_block_to_free_list(block);
}
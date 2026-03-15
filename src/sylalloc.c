#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "sylalloc.h"

#define MIN_SPLIT_SIZE 8
#define ARENA_INIT_SIZE 1024

// track allocations for re-use
static memheader_t* free_list = NULL;

#ifdef SYL_DEBUG
void dbg_validate_free_list(void) {
    memheader_t* fast = free_list;
    memheader_t* slow = free_list;

    while(fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if(slow == fast) {
            fprintf(stderr, "SYLALLOC ERROR: cycle detected in free list\n");
            abort();
        }
    }

    memheader_t* curr = free_list;
    while(curr) {
        if(!curr->is_free) {
            fprintf(stderr, "SYLALLOC ERROR: block in free list is not marked free\n");
            abort();
        }

        if(curr->size == 0) {
            fprintf(stderr, "SYLALLOC ERROR: zero sized block in free list\n");
            abort();
        }

        if(curr->next && curr > curr->next) {
            fprintf(stderr, "SYLALLOC ERROR: free list not in memory-sorted order\n");
            abort();
        }

        if(curr->next) {
            char* end = (char*)curr + MEMHEADER_SIZE + curr->size;
            if(end == (char*)curr->next) {
                fprintf(stderr, "SYLALLOC ERROR: adjacent blocks are not coalesced\n");
                // abort();
            }
        }

        curr = curr->next;
    }
}
#endif

#ifdef SYL_DEBUG
void dbg_dump_free_list(void) {
    printf("\n============ START DUMP ============\n");
    int idx = 0;
    memheader_t* curr = free_list;
    while(curr) {
        const char* free = curr->is_free ? "yes" : "no";
        void* block_start = (void*)curr;
        void* user_start = (void*)(curr + 1);
        void* block_end = (void*)((char*)curr + curr->size);
        ptrdiff_t gap = 0;
        if(curr->next) {
            gap = (char*)curr->next - ( (char*)curr + MEMHEADER_SIZE + curr->size );
        }

        printf("[%d] free=%s  block=%p  user=%p  size=%zu  end=%p  next=%p  gap=%td\n", 
            idx, free, block_start, user_start, curr->size, block_end, (void*)curr->next, gap);

        idx++;
        curr = curr->next;
    }

    printf("============ END DUMP ============\n");
}
#endif

static inline void* header_to_user_start(memheader_t* block) {
    return (void*)(block + 1);
}

static inline memheader_t* user_start_to_header(void* ptr) {
    return (memheader_t*)ptr - 1;
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

    dbg_validate_free_list();
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

    dbg_validate_free_list();
    dbg_dump_free_list();
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

    if(!block) {
        memheader_t* arena = mmap_alloc(ARENA_INIT_SIZE);
        if(!arena) {
            return NULL;
        }
        block = (memheader_t*)arena;
        block->size = ARENA_INIT_SIZE - MEMHEADER_SIZE;
        add_block_to_free_list(block);
    }

    remove_block_from_free_list(block);
    split_block(block, aligned_size);

    block->is_free = false;
    block->size = aligned_size;
    block->next = NULL;

    return header_to_user_start(block);
}


// merge 2 blocks if they are adjacent to each other in free_list memory
// requires contiguous free_list
static bool merge_if_adjacent(memheader_t* left, memheader_t* right) {
    if(!left || !right) return false;

    char* left_end = (char*)left + MEMHEADER_SIZE + left->size;
    if(left_end == (char*)right) {
        left->size += MEMHEADER_SIZE + right->size;
        left->next = right->next;

        right->next = NULL;
        right->is_free = false;
        return true;
    }
    return false;
}

// coalesce block with neighbors in free_list
// assumes free_list is sorted by memory address
static void coalesce_block(memheader_t* block) {
    if (!block) return;

    // try merge prev and curr block first
    memheader_t* prev = NULL;
    memheader_t* current = free_list;

    while (current && current != block) {
        prev = current;
        current = current->next;
    }

    if (prev && merge_if_adjacent(prev, block)) {
        // for safety, set block pointer to prev
        block = prev;
    }

    while (block->next) {
        // continue merging with the next if physically contiguous
        if(!merge_if_adjacent(block, block->next)) break;
    }
}

void syl_free(void* ptr) {
    if(!ptr) {
        return;
    }

    memheader_t* block = user_start_to_header(ptr);

    if(block->is_free) {
        return;
    }

    add_block_to_free_list(block);
    coalesce_block(block);

    dbg_validate_free_list();
    dbg_dump_free_list();
}
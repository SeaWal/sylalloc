#include <stdio.h>
#include <stdlib.h>

#include "sylalloc.h"
#include "syldebug.h"

void dbg_validate_free_list(memheader_t* free_list) {
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


void dbg_dump_free_list(memheader_t* free_list) {
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

void dbg_validate_all(memheader_t* free_list) {
    dbg_validate_free_list(free_list);
    dbg_dump_free_list(free_list);
}
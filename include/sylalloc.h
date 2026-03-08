#ifndef SYLALLOC_H
#define SYLALLOC_H

#include <stddef.h>
#include <stdbool.h>

typedef struct memheader_t {
    size_t size;
    bool is_free;
    struct memheader_t* next;
} memheader_t;

#define MEMHEADER_SIZE sizeof(memheader_t)

void* syl_malloc(size_t size);
void syl_free(void* ptr);

#ifdef SYL_DEBUG
void dbg_validate_free_list(void);
void dbg_dump_free_list(void);
#else
#define dbg_validate_free_list() ((void)0)
#define dbg_dump_free_list() ((void)0)
#endif

#endif
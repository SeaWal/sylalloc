#ifndef SYLALLOC_H
#define SYLALLOC_H

#include <stddef.h>
#include <stdbool.h>

typedef struct memheader_t {
    size_t size;
    bool is_free;
    struct memheader_t* next;
} memheader_t;

void* syl_malloc(size_t size);

#endif
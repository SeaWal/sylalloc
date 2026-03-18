#ifndef SYLALLOC_DEBUG_H
#define SYLALLOC_DEBUG_H

#include "sylalloc.h"

void dbg_validate_free_list(memheader_t* free_list);
void dbg_dump_free_list(memheader_t* free_list);
void dbg_validate_all(memheader_t* free_list);

#endif

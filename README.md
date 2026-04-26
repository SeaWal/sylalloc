# sylalloc (See Ya Later, ALLOCator)

**sylalloc** is a small heap allocator written C, intended mainly as a learning
exercise. It implements custom versions of `malloc`, `free`, obtaining new memory 
from the OS using `mmap`.

**sylalloc** is *not* intended to be a production-quality allocator (not 
thread-safe or cross-platform, etc.). It's basically just intended to learn
how memory management works under the hood.

The goal is to:
* Understand how malloc/free work internally
* Build a (decently) working allocator from scratch using free-lists
* Explore fragmentation, reuse, etc.
* View allocations/deallocations using simple tracing

## Design Overview
**sylalloc** manages memory using:
* A free list (singly linked list of free blocks)
* Block headers (memheader_t) to store metadata
* Memory obtained from the OS via mmap
* First-fit allocation strategy
* Block splitting and coalescing to reuse memory and avoid fragmentation

Each allocation is preceded by a header:

```
+---------------------+----------------------+
|   memheader_t       |    User Memory       |
+---------------------+----------------------+
^                     ^
|                     |
block pointer         returned pointer
```

where the header is defined as:

```c
typedef struct memheader_t {
    size_t size;              // size of user memory
    bool is_free;             // free or allocated
    struct memheader_t* next; // next free block
} memheader_t;
```

## Arenas

Memory is requested from the OS using `mmap`.

Allocator uses:

```
arena_size = max(requested_size + header, ARENA_INIT_SIZE)
```

This allows:

* Small allocations → reuse larger arena
* Large allocations → dedicated arena

---

## Allocation (`syl_malloc`)

1. Align requested size to `max_align_t`
2. Search free list using [**first-fit**](https://courses.grainger.illinois.edu/cs240/fa2020/notes/heapMemoryAllocation.html#:~:text=Strategy%20%231%3A%20First%2DFit)
3. If found:
   * Remove block from free list
   * Split if large enough
4. If not found:
   * Request memory from OS (`mmap`)
   * Add as a new free block
5. Return pointer to user memory

---

## Free  (`syl_free`)

1. Convert user pointer → block header
2. Add block back to free list (sorted by address)
3. Attempt to **coalesce with neighbors**

---

## Block Splitting

When a block is larger than needed:

```
Before:
[ BLOCK size=1000 ]

After allocating 200:
[ USED 200 ][ FREE 776 ]
```

Conditions:

* Only split if remaining size can hold:

  * a header
  * a minimum payload (`MIN_SPLIT_SIZE`)

---

## Coalescing

Adjacent free blocks are merged to reduce fragmentation:

```
Before:
[ FREE 100 ][ FREE 200 ]

After:
[ FREE 324 ]
```

Requirements:

* Free list must be **sorted by memory address**
* Blocks must be **physically adjacent**

---

# 🐞 Debugging & Validation

`sylalloc` includes optional debug tools:

## Enable Debug Mode

Compile with:

```bash
-DSYL_DEBUG
```

## Features

### Free List Validation

Checks:

* list is sorted
* no overlapping blocks
* all blocks are marked free


## Tracing (SYL_TRACE)

Enable:

```bash
-DSYL_TRACE
```

Logs allocator events via user-provided logger:

```
[TRACE] [ALLOC] request=32
[TRACE] [SPLIT] block=0x1000 alloc=32 remainder=968
[TRACE] [FREE ] block=0x1000
[TRACE] [COAL ] merge 0x1000 + 0x1020
```

---

## Logger Integration

Allocator does not own logging.

Instead:

```c
void syl_set_logger(struct Logger* logger);
```

User provides logger:

```c
struct Logger logger;
log_init(&logger, "trace.log");
syl_set_logger(&logger);
```

---

## Known Limitations

* Not thread-safe
* No `realloc` or `calloc`
* No boundary tags (coalescing requires list traversal)
* Single free list → O(n) allocation
# sylalloc (See Ya Later, ALLOCator)

**sylalloc** is a small heap allocator written C, intended mainly as a learning
exercise. It implements custom versions of `malloc`, `free`, and `realloc` on
top of `mmap`.

**sylalloc** is *not* intended to be a production-quality allocator. It is not 
thread-safe or cross-platform.

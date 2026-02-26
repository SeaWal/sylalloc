#include <stdio.h>
#include "sylalloc.h"

#define FAIL 0
#define PASS 1

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

#define TEST_INIT(name) \
    printf("%s :: ", name);

#define TEST_END(result) \
    if(result == PASS) printf(COLOR_GREEN "PASS\n" COLOR_RESET); \
    else printf(COLOR_RED "FAIL\n" COLOR_RESET); \
    return;

#define ASSERT(condition) \
    if(!(condition)) { \
        TEST_END(FAIL) \
    }

static void test_basic_alloc() {
    TEST_INIT("test_basic_alloc");

    void* p = syl_malloc(32);
    ASSERT(p != NULL);

    syl_free(p);

    TEST_END(PASS);
}

static void test_free() {
    TEST_INIT("test_free");

    void* p = syl_malloc(32);
    syl_free(p);
    ASSERT(p == NULL);

    TEST_END(PASS);
}

static void test_alloc_zero_returns_null() {
    TEST_INIT("test_alloc_zero_returns_null");

    void* p = syl_malloc(0);
    ASSERT(p == NULL);

    syl_free(p);

    TEST_END(PASS);
}

static void test_free_null_no_crash() {
    TEST_INIT("test_free_null_no_crash");

    syl_free(NULL);

    TEST_END(PASS);
}

static void test_reuse_freed_mem() {
    TEST_INIT("test_reuse_freed_mem");

    void* p1 = syl_malloc(32);
    ASSERT(p1 != NULL);
    syl_free(p1);

    void* p2 = syl_malloc(32);
    ASSERT(p2 != NULL);
    ASSERT(p1 == p2);

    syl_free(p2);

    TEST_END(PASS);
}


int main() {
    test_basic_alloc();
    test_free();
    test_alloc_zero_returns_null();
    test_free_null_no_crash();
    test_reuse_freed_mem();
}
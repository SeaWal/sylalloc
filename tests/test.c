#include <stdalign.h>
#include <stdio.h>
#include "sylalloc.h"

#define FAIL 0
#define PASS 1

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

static unsigned int num_tests = 0;
static unsigned int num_passed = 0;

#define TEST_INIT(name) \
    num_tests++; \
    printf("%s :: ", name);

#define TEST_END(result) \
    do { \
        if(result == PASS) { \
            num_passed++; \
            printf(COLOR_GREEN "PASS\n" COLOR_RESET); \
        } \
        else printf(COLOR_RED "FAIL\n" COLOR_RESET); \
        return; \
    } while(0);

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

static void test_double_free_no_crash() {
    TEST_INIT("test_double_free_no_crash");

    void* p = syl_malloc(32);
    syl_free(p);
    syl_free(p);

    TEST_END(PASS);
}

static void test_alignment() {
    TEST_INIT("test_alignment");

    size_t max_size = 256;
    for(size_t size = 1; size < max_size; size++) {
        void* p = syl_malloc(size);
        ASSERT(p != NULL);
        ASSERT( ((uintptr_t)p % alignof(max_align_t)) == 0);
        syl_free(p);
    }

    TEST_END(PASS);
}

static void test_allocated_mem_is_writeable() {
    TEST_INIT("test_allocated_mem_is_writeable");

    char* p = syl_malloc(32);
    ASSERT(p != NULL);

    for(int i = 0; i < 32; i++) {
        p[i] = (char)i;
    }

    for(int i = 0; i < 32; i++) {
        ASSERT(p[i] == (char)i);
    }

    syl_free(p);

    TEST_END(PASS);
}

static void test_alloc_mem_no_overlap() {
    TEST_INIT("test_alloc_mem_no_overlap");

    char* a = syl_malloc(32);
    char* b = syl_malloc(32);

    for(int i = 0; i < 32; i++) {
        a[i] = 'a';
        b[i] = 'b';
    }

    for(int i = 0; i < 32; i++) {
        ASSERT(a[i] == 'a');
        ASSERT(b[i] == 'b');
    }

    syl_free(a);
    syl_free(b);

    TEST_END(PASS);
}

static void test_split_alloc() {
    TEST_INIT("test_split_alloc");

    void* p = syl_malloc(128);
    syl_free(p);

    void* q = syl_malloc(32);
    ASSERT(q == p);

    syl_free(q);

    TEST_END(PASS);
}

int main() {
    printf("============ SYLALLOC TESTS START ============\n\n");

    test_basic_alloc();
    test_alloc_zero_returns_null();
    test_free_null_no_crash();
    test_reuse_freed_mem();
    test_double_free_no_crash();
    test_alignment();
    test_allocated_mem_is_writeable();
    test_alloc_mem_no_overlap();
    test_split_alloc();

    printf("\nResults: %d/%d tests passed.\n", num_passed, num_tests);
    printf("\n============ SYLALLOC TESTS END ============\n");
}
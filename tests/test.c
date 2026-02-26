#include <stdio.h>

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

    TEST_END(PASS);
}


int main() {
    test_basic_alloc();
}
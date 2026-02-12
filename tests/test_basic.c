#include <assert.h>
#include <stdio.h>

#include "sylalloc.h"

int main() {
    void* p = syl_malloc(32);
    if(p == NULL) {
        printf("failed to allocate\n");
    } else {
        printf("successfully allocated\n");
    }
}
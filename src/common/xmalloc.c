#include <stdlib.h>
#include <stdio.h>

#include "xmalloc.h"

void *xmalloc(size_t size) {
    void *mem = malloc(size);

    if (mem == NULL) {
        fprintf(stderr, "FATAL: Out of Memory!\n");
        exit(EXIT_FAILURE);
    }

    return mem;
}

void *xrealloc(void *ptr, size_t size) {
    void *mem = realloc(ptr, size);

    if (mem == NULL) {
        fprintf(stderr, "FATAL: Out of memory!\n");
        exit(EXIT_FAILURE);
    }

    return mem;
}
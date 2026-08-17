#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/vector.h"
#include "common/xmalloc.h"

#define VECTOR_GROWTH_FACTOR 2

Vector vector_new_with_capacity(size_t element_size, size_t capacity) {
    void *buf = xmalloc(element_size * capacity);
    if (buf == NULL) {
        printf("ERROR: Out Of Memory!");
        exit(EXIT_FAILURE);
    }
    
    return (Vector) {
        .buffer = buf,
        .size = 0,
        .capacity = capacity,
        .element_size = element_size
    };
}

Vector vector_new(size_t element_size) {
    return vector_new_with_capacity(element_size, 8);
}

void vector_push(Vector *vec, const void *element) {
    if (vec->size == vec->capacity) {
        size_t capacity_new = vec->capacity == 0 ? 1 : vec->capacity * VECTOR_GROWTH_FACTOR;
        void *buf_new = xrealloc(vec->buffer, capacity_new * vec->element_size);

        vec->capacity = capacity_new;
        vec->buffer = buf_new;
    }

    memcpy((unsigned char*) vec->buffer + vec->size * vec->element_size, element, vec->element_size);
    vec->size++;
}

void vector_pop(Vector *vec, void *out) {
    if (vec->size == 0) {
        fprintf(stderr, "ERROR: Vector underflow!\n");
        return;
    }

    vec->size--;
    memcpy(out, (unsigned char *) vec->buffer + (vec->size) * vec->element_size, vec->element_size);
}

void *vector_at(Vector *vec, size_t index) {
    if (index >= vec->size) {
        fprintf(stderr, "ERROR: Out of bounds access!\n");
        return NULL;
    }

    return (unsigned char*)vec->buffer + index * vec->element_size;
}

const void *vector_at_const(const Vector *vec, size_t index) {
    if (index >= vec->size) {
        fprintf(stderr, "ERROR: Out of bounds access!\n");
        return NULL;
    }

    return (const unsigned char*)vec->buffer + index * vec->element_size;
}

void vector_free(Vector *vec) {
    free(vec->buffer);
    vec->buffer = NULL;
    vec->size = 0;
    vec->capacity = 0;
    vec->element_size = 0;
}
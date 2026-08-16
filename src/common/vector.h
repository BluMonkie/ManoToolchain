#pragma once

#include <stddef.h>

#define VECTOR_GROWTH_FACTOR 2

typedef struct {
    void *buffer;
    size_t size;
    size_t capacity;
    size_t element_size;
} Vector;

Vector vector_new_with_capacity(size_t element_size, size_t capacity);
Vector vector_new(size_t element_size);

void vector_push(Vector *vec, const void *element);
void vector_pop(Vector *vec, void *out);

void *vector_at(Vector *vec, size_t index);
const void *vector_at_const(const Vector *vec, size_t index);

void vector_free(Vector *vec);
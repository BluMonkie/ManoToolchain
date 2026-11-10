#pragma once

#include <stddef.h>

#include "vector.h"

#define DEFAULT_CHUNK_SIZE (4u * 1024u * 1024u)
// #define MAX_CHUNK_SIZE 4 * 1024 * 1024
#define arena_push(arena_ptr, type, value_expr) \
    ((type *)arena_alloc_type((arena_ptr), sizeof(type), &(type[]) { value_expr }))

typedef struct {
    Vector chunks;
    size_t current_chunk;
} Arena;

Arena arena_new();

void *arena_alloc(Arena *arena, size_t size);
void *arena_alloc_type(Arena *arena, size_t size, void* data);
void arena_drop(Arena *arena);
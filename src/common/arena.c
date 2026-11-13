#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#include "arena.h"
#include "xmalloc.h"

typedef struct {
    void *buffer;
    size_t index;
    size_t capacity;
} ArenaChunk;

static size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

static ArenaChunk chunk_new(size_t capacity) {
    return (ArenaChunk) {
        .buffer = xmalloc(capacity),
        .capacity = capacity,
        .index = 0,
    };
}

Arena arena_new() {
    Vector chunks = vector_new(sizeof(ArenaChunk));
    ArenaChunk chunk = chunk_new(DEFAULT_CHUNK_SIZE);
    vector_push(&chunks, &chunk);
    
    return (Arena) {
        .chunks = chunks,
        .current_chunk = 0,
    };
}

void *arena_alloc(Arena *arena, size_t size) {
    ArenaChunk *chunk = vector_at(&arena->chunks, arena->current_chunk);
    size_t alignment = alignof(max_align_t);
    size = ((size + alignment - 1) / alignment) * alignment;

    if ((chunk->capacity - chunk->index) < size) {
        ArenaChunk new_chunk = chunk_new(max(size, DEFAULT_CHUNK_SIZE));
        vector_push(&arena->chunks, &new_chunk);
        arena->current_chunk++;

        chunk = vector_at(&arena->chunks, arena->current_chunk);
    }

    size_t temp = chunk->index;
    chunk->index += size;
    return ((unsigned char *) chunk->buffer) + temp;
}

void *arena_alloc_type(Arena *arena, size_t size, const void* data) {
    void *ptr = arena_alloc(arena, size);
    memcpy(ptr, data, size);
    return ptr;
}

void arena_drop(Arena *arena) {
    for (size_t i = 0; i <= arena->current_chunk; i++) {
        ArenaChunk *chunk = vector_at(&arena->chunks, i);
        xfree(chunk->buffer);
    }

    vector_free(&arena->chunks);
}
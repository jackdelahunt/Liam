#pragma once
#include <stdlib.h>
#include "macros.h"

#define ALLOC(size) \
    alloc_arena(current__working__arena__, size)

typedef struct Arena {
    void* buffer;
    size_t size;
    size_t base;
} Arena;

Arena* current__working__arena__ = NULL;

Arena make_arena(size_t size) {
    return (Arena){malloc(size), size, 0};
}

void* alloc_arena(Arena* arena, size_t size) {
    ASSERT(current__working__arena__, "No arena currently set");
    void* ptr = arena->buffer + arena->base;
    arena->base += size;
    assert(arena->base < arena->size);
    return ptr;
}

void free_arena(Arena* arena) {
    free(arena->buffer);
    arena->buffer = NULL;
    if(current__working__arena__ == arena) {
        current__working__arena__ = NULL;
    }
}
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

extern Arena* current__working__arena__;

Arena make_arena(size_t size);
void* alloc_arena(Arena* arena, size_t size);
void free_arena(Arena* arena);
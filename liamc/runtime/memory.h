#pragma once

#include "core.h"
#include <stdlib.h>

struct Allocator {
    virtual void *alloc(u64 size)   = 0;
    virtual void destroy(void *ptr) = 0;
};

static Allocator *allocator = NULL;

struct Malloc : public Allocator {
    void *alloc(u64 size) override;
    void destroy(void *ptr) override;
};

template <typename T> T *alloc(T t) {
    auto ptr = (T *)allocator->alloc(sizeof(T));
    *ptr     = t;
    return ptr;
}

template <typename T> void destroy(T *t) {
    allocator->destroy((void *)t);
}

void set_allocator(str name, u64 size);

template <typename T> u64 hashcode(T *t) {
    return u64{(uint64_t)t};
}
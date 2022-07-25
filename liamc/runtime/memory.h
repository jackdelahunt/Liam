#pragma once

#include "core.h"
#include <iostream>
#include <stdlib.h>

namespace Internal {
struct Allocator {
    virtual void *alloc(u64 size)                    = 0;
    virtual void *re_alloc(void *data, u64 new_size) = 0;
    virtual void destroy(void *ptr)                  = 0;
};

static Allocator *allocator = NULL;
} // namespace Internal

struct Malloc : public Internal::Allocator {
    void *alloc(u64 size) override;
    void *re_alloc(void *data, u64 new_size) override;
    void destroy(void *ptr) override;
};

template <typename T> T *alloc(T t) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));

    auto ptr = (T *)Internal::allocator->alloc(sizeof(T));
    *ptr     = t;
    return ptr;
}

template <typename T> T *re_alloc(T *data, u64 new_size) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));
    return Internal::allocator->re_alloc(data, new_size);
}

template <typename T> void destroy(T *t) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));
    Internal::allocator->destroy((void *)t);
}

void set_allocator(str name, u64 size);

template <typename T> u64 hashcode(T *t) {
    return u64{(uint64_t)t};
}
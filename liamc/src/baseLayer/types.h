#pragma once

#include <cstdint>

#include "debug.h"

typedef int8_t  i8;
typedef uint8_t u8;

typedef int16_t  i16;
typedef uint16_t u16;

typedef int32_t  i32;
typedef uint32_t u32;
typedef float    f32;

typedef int64_t     i64;
typedef uint64_t    u64;
typedef long double f64;

struct Allocator {
    virtual void *alloc(u64 size)                  = 0;
    virtual void  free(void *ptr)                  = 0;
    virtual void *realloc(void *ptr, u64 new_size) = 0;
};

template <typename T> struct Option {
    explicit Option() {
        this->t     = {};
        this->valid = false;
    }

    explicit Option(T t) {
        this->t     = t;
        this->valid = true;
    }

    bool is_some() {
        return valid;
    }

    T value() {
        ASSERT(this->valid);
        return this->t;
    }

  private:
    T    t;
    bool valid;
};

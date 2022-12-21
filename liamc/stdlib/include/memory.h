#pragma once

#include "core.h"
#include <iostream>
#include <stdlib.h>

template <typename T> T *alloc(T t) {
    auto ptr = (T *)malloc(sizeof(T));
    *ptr     = t;
    return ptr;
}

template <typename T> T *re_alloc(T *data, u64 new_size) {
    return (T *)realloc(data, new_size);
}

template <typename T> void destroy(T *t) {
    free(t);
}

template <typename T> u64 hashcode(T *t) {
    return u64{(uint64_t)t};
}
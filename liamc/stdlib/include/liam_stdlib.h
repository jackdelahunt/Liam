#pragma once

#include <core.h>
#include <iostream>
#include <stdlib.h>

namespace stdlib {
template <typename T> void println(const T &t) {
    std::cout << t << "\n";
}

template <typename T> LiamInternal::PointerSlice<T> alloc(u64 size) {
    return LiamInternal::PointerSlice<T>{.size = size, .ptr = (T *)malloc(size)};
}

template <typename T> u64 ptr_len(LiamInternal::PointerSlice<T> ptr) {
    return ptr.size;
}

u64 str_len(const str &s);
str substr(str s, u64 start, u64 length);
str char_at(str s, u64 index);
}; // namespace stdlib

#include "liam_stdlib.cpp"
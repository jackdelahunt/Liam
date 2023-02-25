#include "core.h"
#include <iostream>

template <typename T> void println(const T &t) {
    std::cout << t << "\n";
}

u64 len(const str &s);
str substr(str s, u64 start, u64 length);
str char_at(str s, u64 index);

#include "stblib.h"

u64 len(const str &s) {
    return s.length;
}

str substr(str s, u64 start, u64 length) {

    if (start + length > s.length)
    { panic(LiamInternal::make_str("Panic :: out of range substr")); }
    return str{.chars = &s.chars[start], .length = length};
}

str char_at(str s, u64 index) {
    if (index >= s.length)
    { panic(LiamInternal::make_str("Panic :: out of range char_at")); }
    return str{.chars = &s.chars[index], .length = 1};
}
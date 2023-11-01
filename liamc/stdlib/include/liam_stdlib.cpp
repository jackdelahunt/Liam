#include "liam_stdlib.h"

namespace stdlib {
u64 str_len(const str &s) {
    return s.length;
}

str substr(str s, u64 start, u64 length) {

    if (start + length > s.length)
    { __panic(LiamInternal::make_str("Panic :: out of range substr")); }
    return str{.chars = &s.chars[start], .length = length};
}

str char_at(str s, u64 index) {
    if (index >= s.length)
    { __panic(LiamInternal::make_str("Panic :: out of range char_at")); }
    return str{.chars = &s.chars[index], .length = 1};
}
}; // namespace stdlib
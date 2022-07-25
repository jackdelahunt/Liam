#include "core.h"
#include <string.h>

u8 _u8(u8 n) {
    return n;
}

s8 _s8(s8 n) {
    return n;
}

u32 _u32(u32 n) {
    return n;
}

s32 _s32(s32 n) {
    return n;
}

f32 _f32(f32 n) {
    return n;
}

u64 _u64(u64 n) {
    return n;
}

s64 _s64(s64 n) {
    return n;
}

f64 _f64(f64 n) {
    return n;
}

std::string str::pretty_string(std::string indentation) {
    char *cpy = (char *)malloc(sizeof(char) * length);
    memcpy(cpy, chars, length);
    cpy[length] = '\0';
    auto print  = indentation + std::string(cpy);
    free(cpy);
    return print;
}

bool str::compare_c_str(const char *c_str) {
    for (u64 i = 0; i < this->length; i++)
    {
        if (c_str[i] == '\0')
            return false;

        if (chars[i] == c_str[i])
        {}
        else
            return false;
    }

    return true;
}

bool operator==(const str &l, const str &r) {
    if (l.length != r.length)
        return false;

    for (uint64_t i = 0; i < l.length; i++)
    {
        if (l.chars[i] == r.chars[i])
        {}
        else
            return false;
    }

    return true;
}

str make_str(char *chars, uint64_t length) {
    return str{chars, length};
}

std::ostream &operator<<(std::ostream &os, const str &obj) {
    for (int i = 0; i < obj.length; i++)
    { os << obj.chars[i]; }
    return os;
}

u64 len(const str &s) {
    return s.length;
}

str substr(str s, u64 start, u64 length) {

    panic_if(start + length > s.length, Internal::make_str("Panic :: out of range substr"));

    return str{.chars = &s.chars[start], .length = length};
}

void panic_if(bool condition, str message) {
    if (condition)
    {
        std::cout << message << "\n";
        exit(1);
    }
}

namespace Internal {
str make_str(const char *c_str) {
    u64 length = strlen(c_str);
    return str{.chars = (char *)c_str, .length = length};
}
} // namespace Internal
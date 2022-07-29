#include "builtin_string.h"
#include "memory.h"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const String &obj) {
    os << obj.string;
    return os;
}

String make_string(str s) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));

    if (s.length == 0)
    { return String{.string = str{.chars = NULL, .length = 0}, .size = 0}; }

    u64 size   = s.length + 50;
    char *data = (char *)Internal::allocator->alloc(sizeof(char) * size);

    memcpy(data, s.chars, s.length);

    return String{.string = str{.chars = data, .length = s.length}, .size = size};
}

void string_append(String *s, String *x) {
    if (s->string.length + x->string.length > s->size)
    {
        u64 new_size = s->size + x->string.length + 50;
        Internal::allocator->re_alloc(s->string.chars, new_size);
        s->size = new_size;
    }

    memcpy(s->string.chars + s->string.length, x->string.chars, x->string.length);
    s->string.length += x->string.length;
}

void string_append_str(String *string, str s) {
    if (string->string.length + s.length > string->size)
    {
        u64 new_size = string->size + s.length + 50;
        Internal::allocator->re_alloc(string->string.chars, new_size);
        string->size = new_size;
    }

    memcpy(string->string.chars + string->string.length, s.chars, s.length);
    string->string.length += s.length;
}
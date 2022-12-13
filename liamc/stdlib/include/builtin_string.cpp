#include "builtin_string.h"
#include "memory.h"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const String &obj) {
    os << obj.string;
    return os;
}

String make_string(str s) {
    if (Internal::allocator == NULL)
    { panic(Internal::make_str("Allocator not set")); }

    if (s.length == 0)
    { return String{.string = str{.chars = NULL, .length = 0}, .size = 0}; }

    u64 size   = s.length + 50;
    char *data = (char *)Internal::allocator->alloc(sizeof(char) * size);

    memcpy(data, s.chars, s.length);

    return String{.string = str{.chars = data, .length = s.length}, .size = size};
}

void __append(String *self, String *x) {
    if (self->string.length + x->string.length > self->size)
    {
        u64 new_size = self->size + x->string.length + 50;
        Internal::allocator->re_alloc(self->string.chars, new_size);
        self->size = new_size;
    }

    memcpy(self->string.chars + self->string.length, x->string.chars, x->string.length);
    self->string.length += x->string.length;
}

void __append_str(String *self, str s) {
    if (self->string.length + s.length > self->size)
    {
        u64 new_size = self->size + s.length + 50;
        Internal::allocator->re_alloc(self->string.chars, new_size);
        self->size = new_size;
    }

    memcpy(self->string.chars + self->string.length, s.chars, s.length);
    self->string.length += s.length;
}
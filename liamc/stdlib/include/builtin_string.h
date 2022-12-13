#pragma once

#include "core.h"
#include <string>

struct String {
    str string;
    u64 size;
};

std::ostream &operator<<(std::ostream &os, const String &obj);

String make_string(str s);
void __append(String *s, String *x);
void __append_str(String *string, str s);
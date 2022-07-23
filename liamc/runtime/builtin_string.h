#pragma once

#include "core.h"
#include <string>

struct String {
    std::string string;

    std::string pretty_string(std::string indentation);
};

String make_string();
String make_string_from(str s);
str to_str(String *s);
void string_append(String *s, String *x);
str string_substring(String *string, u64 start, u64 length);
u64 string_length(String *string);
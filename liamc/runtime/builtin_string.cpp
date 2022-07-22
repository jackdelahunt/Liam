#include "String.h"

std::string String::pretty_string(std::string indentation) {
    return indentation + string;
}

String make_string() {
    return String{.string = std::string()};
}

String make_string_from(str s) {
    return String{.string = std::string(s.chars)};
}

str to_str(String *s) {
    return make_str((char *)s->string.c_str(), s->string.size());
}

void string_append(String *s, String *x) {
    s->string.append(x->string);
}

str string_substring(String *string, u64 start, u64 length) {
    return make_str((char *)&string->string.c_str()[start], length);
}

u64 string_length(String *string) {
    return string->string.size();
}
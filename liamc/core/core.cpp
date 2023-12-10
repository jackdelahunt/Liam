#include "core.h"
#include <string.h>

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
        if (l.chars[i] != r.chars[i])
            return false;
    }

    return true;
}

std::ostream &operator<<(std::ostream &os, const str &obj) {
    for (int i = 0; i < obj.length; i++)
    {
        os << obj.chars[i];
    }
    return os;
}

namespace Liam {
str make_str(char *chars, uint64_t length) {
    return str{chars, length};
}

str make_str(const char *c_str) {
    u64 length = strlen(c_str);
    return str{.chars = (char *)c_str, .length = length};
}
} // namespace Liam

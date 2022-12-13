#include "os.h"
#include <string.h>

namespace Internal {
u32 argc    = 0;
char **argv = NULL;
} // namespace Internal

Array<str> argv() {
    auto array = make_array<str>();

    for (u32 i = 0; i < Internal::argc; i++)
    { __append(&array, make_str(Internal::argv[i], strlen(Internal::argv[i]))); }

    return array;
}

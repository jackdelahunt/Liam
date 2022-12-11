#include "io.h"
#include "memory.h"

namespace Internal {
std::tuple<u64, char *> read_file(char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    { panic(make_str("Cannot open file")); }

    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    rewind(fp);

    if (Internal::allocator == NULL)
    { panic(make_str("Need to set allocator before reading file")); }
    char *buffer = (char *)Internal::allocator->alloc(lSize);

    if (1 != fread(buffer, lSize, 1, fp))
    {
        fclose(fp);
        free(buffer);
        fputs("entire read fails", stderr);
        exit(1);
    }

    fclose(fp);
    return {lSize, buffer};
}
} // namespace Internal

String read(str path) {
    auto [length, buffer] = Internal::read_file(path.chars);
    return make_string(str{.chars = buffer, .length = length});
}
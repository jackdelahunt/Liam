#include "memory.h"

namespace Internal {
Allocator *allocator = NULL;
}

void *Malloc::alloc(u64 size) {
    return malloc(size);
}

void *Malloc::re_alloc(void *data, u64 new_size) {
    return realloc(data, new_size);
}

void Malloc::destroy(void *ptr) {
    free(ptr);
}

void set_allocator(str name, u64 size) {
    if (name.compare_c_str("malloc"))
    { Internal::allocator = new Malloc{}; }
}
#include "memory.h"

void *Malloc::alloc(u64 size) {
    return malloc(size);
}

void Malloc::destroy(void *ptr) {
    free(ptr);
}

void set_allocator(str name, u64 size) {
    if (name.compare_c_str("malloc"))
    { allocator = new Malloc{}; }
}

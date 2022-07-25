#include "memory.h"

void *Malloc::alloc(u64 size) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));
    return malloc(size);
}

void *Malloc::re_alloc(void *data, u64 new_size) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));
    return realloc(data, new_size);
}

void Malloc::destroy(void *ptr) {
    panic_if(Internal::allocator == NULL, Internal::make_str("Allocator not set"));
    free(ptr);
}

void set_allocator(str name, u64 size) {
    if (name.compare_c_str("malloc"))
    { Internal::allocator = new Malloc{}; }
}
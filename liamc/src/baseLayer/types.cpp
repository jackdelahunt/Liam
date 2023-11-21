#include "types.h"
#include "debug.h"

#include <cstdlib>

ScratchAllocator::ScratchAllocator(u64 size) {
    this->used = 0;
    this->size = size;
    this->data = malloc(size);
}

void *ScratchAllocator::alloc(u64 size) {
    u64 returning_ptr = (u64)this->data + this->used;
    this->used += size;
    ASSERT_MSG(this->used <= this->size, "jack");
    return (void *)returning_ptr;
}

void ScratchAllocator::free(void *ptr) {
}

void *ScratchAllocator::realloc(void *ptr, u64 new_size) {
    return ptr;
}

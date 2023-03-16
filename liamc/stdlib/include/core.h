#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <variant>

#define __panic(message)                                                                                               \
    std::cout << "PANIC " << __FILE__ << " (" << __LINE__ << ") :: " << message << "\n";                               \
    exit(1);

typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t u16;
typedef int16_t i16;

typedef int32_t i32;
typedef uint32_t u32;
typedef float f32;

typedef uint64_t u64;
typedef int64_t i64;
typedef double f64;

struct str {
    char *chars;
    u64 length;

    bool compare_c_str(const char *c_str);
    friend bool operator==(const str &l, const str &r);
};

std::ostream &operator<<(std::ostream &os, const str &obj);

namespace LiamInternal {
u8 __u8(u8 n);
i8 __i8(i8 n);
u32 __u32(u32 n);
i32 __i32(i32 n);
f32 __f32(f32 n);
u64 __u64(u64 n);
i64 __i64(i64 n);
f64 __f64(f64 n);

template <typename T> struct PointerSlice {
    u64 size;
    T *ptr;

    T subscript(u64 index) {
        return this->ptr[index];
    }

    void set(u64 index, T t) {
        this->ptr[index] = t;
    }
};

/*
    str functions, str is declared outside of namespace as it is not interal only
*/
str make_str(char *chars, uint64_t length);
str make_str(const char *c_str);
} // namespace LiamInternal

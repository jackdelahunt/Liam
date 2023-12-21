#pragma once

#include <cstdint>
#include <initializer_list>
#include <iostream>

#define __panic(message)                                                                                               \
    std::cout << "PANIC " << __FILE__ << " (" << __LINE__ << ") :: " << message << "\n";                               \
    exit(1);

typedef uint8_t u8;
typedef int8_t  i8;

typedef uint16_t u16;
typedef int16_t  i16;

typedef int32_t  i32;
typedef uint32_t u32;
typedef float    f32;

typedef uint64_t    u64;
typedef int64_t     i64;
typedef long double f64;

#define __ASSERT(expr)                                                                                                 \
    if (!(expr)) {                                                                                                     \
        std::cerr << "ASSERT :: " << __FILE_NAME__ << " :: line " << __LINE__ << "\n  --> (" << std::string(#expr)     \
                  << ")\n";                                                                                            \
        exit(1);                                                                                                       \
    }

struct str {
    char *chars;
    u64   length;

    bool        compare_c_str(const char *c_str);
    friend bool operator==(const str &l, const str &r);
};

std::ostream &operator<<(std::ostream &os, const str &obj);

namespace Liam {
constexpr u8 make_u8(u8 n) {
    return n;
}

constexpr i8 make_i8(i8 n) {
    return n;
}

constexpr u32 make_u32(u32 n) {
    return n;
}

constexpr i32 make_i32(i32 n) {
    return n;
}

constexpr f32 make_f32(f32 n) {
    return n;
}

constexpr u64 make_u64(u64 n) {
    return n;
}

constexpr i64 make_i64(i64 n) {
    return n;
}

constexpr f64 make_f64(f64 n) {
    return n;
}

template <typename T> struct Slice {
    T  *data;
    u64 size;

    Slice(T *data, u64 size) {
        this->data = data;
        this->size = size;
    }

    Slice<T> slice_full() {
        return Slice<T>(&this->data[0], this->size);
    }

    Slice<T> slice_with_start_and_end(u64 start, u64 end) {
        return Slice<T>(&this->data[start], end - start);
    }

    Slice<T> slice_with_start(u64 start) {
        return Slice<T>(&this->data[start], this->size - start);
    }

    Slice<T> slice_with_end(u64 end) {
        return Slice<T>(&this->data[0], end);
    }

    // we need to use T& to make Slice[n] assignable, check array for more
    // detailed explaination
    T &operator[](u64 index) {
        // TODO we should have bounds checking in this for debug builds
        return this->data[index];
    }

    // define a << overload for std::cout
    friend std::ostream &operator<<(std::ostream &os, const Slice<T> &obj) {
        os << "[";
        for (u64 i = 0; i < obj.size; i++) {
            os << obj.data[i];
            if (i != obj.size - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
};

template <u64 N, typename T> struct StaticArray {
    T   array[N];
    u64 size;

    StaticArray() {
        this->size = N;
    }

    explicit StaticArray(std::initializer_list<T> list) {
        this->size = N;

        u64 index  = 0;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            this->array[index] = *iter;
            index++;
        }
    }

    Slice<T> slice_full() {
        return Slice<T>(&this->array[0], this->size);
    }

    Slice<T> slice_with_start_and_end(u64 start, u64 end) {
        return Slice<T>(&this->array[start], end - start);
    }

    Slice<T> slice_with_start(u64 start) {
        return Slice<T>(&this->array[start], this->size - start);
    }

    Slice<T> slice_with_end(u64 end) {
        return Slice<T>(&this->array[0], end);
    }

    // we need to use T& to make Array[n] assignable i.e. we need a
    // lvalue as just using rvalue will not work. This si fine but
    // is not represented in the compiler so it is kind of magic
    // generated cpp code as the compiler doesn't know about
    // l or r values
    T &operator[](u64 index) {
        // TODO we should have bounds checking in this for debug builds
        return this->array[index];
    }

    friend std::ostream &operator<<(std::ostream &os, const StaticArray<N, T> &obj) {
        os << "[";
        for (u64 i = 0; i < obj.size; i++) {
            os << obj.array[i];
            if (i != obj.size - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
};

/*
    str functions, str is declared outside of namespace as it is not interal only
*/
str make_str(char *chars, uint64_t length);
str make_str(const char *c_str);
} // namespace Liam

#include "core.cpp"

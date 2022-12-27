#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <variant>

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

u8 __u8(u8 n);
i8 __i8(i8 n);
u32 __u32(u32 n);
i32 __i32(i32 n);
f32 __f32(f32 n);
u64 __u64(u64 n);
i64 __i64(i64 n);
f64 __f64(f64 n);

template <typename T> struct __Slice {

    u64 length;
    const T *data_ptr;

    __Slice(std::initializer_list<T> list) {
        this->length   = list.size();
        this->data_ptr = data(list);
    }

    __Slice(T *data_ptr, u64 length) {
        this->length   = length;
        this->data_ptr = data_ptr;
    }

    T operator[](int index) {
        return this->data_ptr[index];
    }
};

struct str {
    char *chars;
    u64 length;

    std::string pretty_string(std::string indentation);
    bool compare_c_str(const char *c_str);
    friend bool operator==(const str &l, const str &r);
};

str make_str(char *chars, uint64_t length);
std::ostream &operator<<(std::ostream &os, const str &obj);

u64 len(const str &s);
str substr(str s, u64 start, u64 length);
str char_at(str s, u64 index);

#define panic(message)                                                                                                 \
    std::cout << "PANIC " << __FILE__ << " (" << __LINE__ << ") :: " << message << "\n";                               \
    exit(1);

template <typename T, typename... Ts> std::ostream &operator<<(std::ostream &os, const std::variant<T, Ts...> &obj) {
    os << "<?>";
    return os;
}

#define __PROPAGATE(type, expr, __VA_ARGS__)                                                                           \
    ({                                                                                                                 \
        auto __evaluated = (expr);                                                                                     \
        if (auto __temp = std::get_if<type>(&__evaluated))                                                             \
        { return *__temp; }                                                                                            \
        std::variant<__VA_ARGS__> v = Internal::cast_variant<__VA_ARGS__>(__evaluated);                                \
        v;                                                                                                             \
    })

namespace Internal {
str make_str(const char *c_str);

template <typename... newTypes, typename... oldTypes> auto cast_variant(const std::variant<oldTypes...> &var) {
    return std::visit(
        []<typename T>(T &&arg) -> std::variant<newTypes...> {
            if constexpr (std::disjunction_v<std::is_same<std::decay_t<T>, newTypes>...>)
            { return arg; }
            else
            { throw std::bad_variant_access(); }
        },
        var
    );
}
} // namespace Internal
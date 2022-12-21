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

u8 _u8(u8 n);
i8 _i8(i8 n);
u32 _u32(u32 n);
i32 _i32(i32 n);
f32 _f32(f32 n);
u64 _u64(u64 n);
i64 _i64(i64 n);
f64 _f64(f64 n);

struct str {
    char *chars;
    uint64_t length;

    std::string pretty_string(std::string indentation);
    bool compare_c_str(const char *c_str);
    friend bool operator==(const str &l, const str &r);
};

str make_str(char *chars, uint64_t length);
std::ostream &operator<<(std::ostream &os, const str &obj);

u64 len(const str &s);
str substr(str s, u64 start, u64 length);
str char_at(str s, u64 index);
bool is_digit(str s);

#define panic(message)                                                                                                 \
    std::cout << "PANIC " << __FILE__ << " (" << __LINE__ << ") :: " << message << "\n";                               \
    exit(1);

#define assert_true(condition)                                                                                         \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
        std::cout << "Assertion Faild :: ";                                                                            \
        panic(Internal::make_str(#condition));                                                                         \
    }

#define assert_false(condition)                                                                                        \
    if (condition)                                                                                                     \
    {                                                                                                                  \
        std::cout << "Assertion Faild :: ";                                                                            \
        panic(Internal::make_str(#condition));                                                                         \
    }

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
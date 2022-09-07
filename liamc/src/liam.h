#pragma once

#include <stdint.h>

#include <string>

typedef int8_t s8;
typedef uint8_t u8;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

void panic(const std::string &msg);

#define TIME_START(name) auto name = std::chrono::high_resolution_clock::now();

#define TIME_END(name, message)                                                                                        \
    {                                                                                                                  \
        auto end                                        = std::chrono::high_resolution_clock::now();                   \
        std::chrono::duration<double, std::milli> delta = end - name;                                                  \
        if (args->value<bool>("time"))                                                                                 \
        { std::cout << message << " :: " << delta.count() << "ms\n"; }                                                 \
    }

#define TRY(type, value, func)                                                                                         \
    type value = nullptr;                                                                                              \
    {                                                                                                                  \
        auto [ptr, error] = func;                                                                                      \
        if (error)                                                                                                     \
        { return {nullptr, true}; }                                                                                    \
        value = ptr;                                                                                                   \
    }

#define NAMED_TOKEN(value, type)                                                                                       \
    Token *value = nullptr;                                                                                            \
    {                                                                                                                  \
        auto tuple     = consume_token_of_type(type);                                                                  \
        value          = std::get<0>(tuple);                                                                           \
        auto try_error = std::get<1>(tuple);                                                                           \
        if (try_error)                                                                                                 \
        { return {nullptr, true}; }                                                                                    \
    }

#define WIN(value)                                                                                                     \
    { value, false }

#define TRY_CALL(func)                                                                                                 \
    {                                                                                                                  \
        auto __start = ErrorReporter::error_count();                                                                   \
        func;                                                                                                          \
        if (ErrorReporter::error_count() > __start)                                                                    \
        { return; }                                                                                                    \
    }

#define TRY_CALL_RET(func)                                                                                              \
    func;                                                                                                             \
    if (ErrorReporter::has_error_since_last_check())                                                                    \
    {                                                                                                               \
        return NULL;                                                                                                \
    }

#define TRY_CALL_RETURN(func, ret)                                                                                              \
    func;                                                                                                             \
    if (ErrorReporter::has_error_since_last_check())                                                                    \
    {                                                                                                               \
        return ret;                                                                                                \
    }
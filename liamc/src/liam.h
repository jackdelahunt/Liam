#pragma once

#include <iostream>
#include <stdint.h>

#include <string>

#define USE_ASSERTS

void panic(const std::string &msg);

#define BIT_SET(mask, bit) ((mask & bit) == bit)

#define SET_BIT(mask, bit) mask |= bit

#define TIME_START(name) auto name = std::chrono::high_resolution_clock::now();

#define TIME_END(name, message)                                                                                        \
    {                                                                                                                  \
        auto end                                        = std::chrono::high_resolution_clock::now();                   \
        std::chrono::duration<double, std::milli> delta = end - name;                                                  \
        if (args->value<bool>("time"))                                                                                 \
        {                                                                                                              \
            std::cout << message << " :: " << delta.count() << "ms\n";                                                 \
        }                                                                                                              \
    }

#define TRY(type, value, func)                                                                                         \
    type value = nullptr;                                                                                              \
    {                                                                                                                  \
        auto [ptr, error] = func;                                                                                      \
        if (error)                                                                                                     \
        {                                                                                                              \
            return {nullptr, true};                                                                                    \
        }                                                                                                              \
        value = ptr;                                                                                                   \
    }

#define NAMED_TOKEN(value, type)                                                                                       \
    Token *value = nullptr;                                                                                            \
    {                                                                                                                  \
        auto tuple     = consume_token_of_type(type);                                                                  \
        value          = std::get<0>(tuple);                                                                           \
        auto try_error = std::get<1>(tuple);                                                                           \
        if (try_error)                                                                                                 \
        {                                                                                                              \
            return {nullptr, true};                                                                                    \
        }                                                                                                              \
    }

// call a func and ifthere is an error return with §a given value
// return the value from the func
#define TRY_CALL_RET(func)                                                                                             \
    func;                                                                                                              \
    {                                                                                                                  \
        if (ErrorReporter::has_error_since_last_check())                                                               \
        {                                                                                                              \
            return {};                                                                                                 \
        }                                                                                                              \
    }

// call a void func and if there is an error return
#define TRY_CALL_VOID(func)                                                                                            \
    func;                                                                                                              \
    {                                                                                                                  \
        if (ErrorReporter::has_error_since_last_check())                                                               \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
    }

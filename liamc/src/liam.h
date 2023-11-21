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

#define WIN(value)                                                                                                     \
    { value, false }

// call a func and if there is an error return else get the value from the function
#define TRY_CALL(func)                                                                                                 \
    ({                                                                                                                 \
        auto __start = ErrorReporter::error_count();                                                                   \
        auto __v     = func;                                                                                           \
        if (ErrorReporter::error_count() > __start)                                                                    \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
        __v;                                                                                                           \
    })

// call a func and if there is an error return with a given value
// return the value from the func
#define TRY_CALL_RET(func, ret)                                                                                        \
    ({                                                                                                                 \
        auto __start = ErrorReporter::error_count();                                                                   \
        auto __v     = func;                                                                                           \
        if (ErrorReporter::error_count() > __start)                                                                    \
        {                                                                                                              \
            return ret;                                                                                                \
        }                                                                                                              \
        __v;                                                                                                           \
    })

// call a void func and if there is an error return
#define TRY_CALL_VOID(func)                                                                                            \
    {                                                                                                                  \
        auto __start = ErrorReporter::error_count();                                                                   \
        func;                                                                                                          \
        if (ErrorReporter::error_count() > __start)                                                                    \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
    }

// call a func and if there is an error return with a given value from the function
#define TRY_CALL_RET_VOID(func, ret)                                                                                   \
    ({                                                                                                                 \
        auto __start = ErrorReporter::error_count();                                                                   \
        func;                                                                                                          \
        if (ErrorReporter::error_count() > __start)                                                                    \
        {                                                                                                              \
            return ret;                                                                                                \
        }                                                                                                              \
    })

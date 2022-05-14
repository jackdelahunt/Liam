#pragma once

#define TIME_START(name)                                            \
    auto name = std::chrono::high_resolution_clock::now();

#define TIME_END(name, message)                                     \
    {auto end = std::chrono::high_resolution_clock::now();          \
    std::chrono::duration<double, std::milli> delta = end - name;   \
    std::cout << message << " :: " << delta.count() << "ms\n";}

#define TRY(type, value, func)                                  \
    type value = nullptr;                                       \
     {                                                          \
     auto [ptr, error] = func;                                  \
        if(error) {                                             \
            return {nullptr, true};                             \
        }                                                       \
        value = ptr;                                            \
    }

#define NAMED_TOKEN(value, type) \
    Token* value = nullptr;                                     \
    {                                                           \
        auto tuple = consume_token_of_type(type);               \
        value = std::get<0>(tuple);                             \
        auto try_error = std::get<1>(tuple);                    \
        if(try_error) {                                         \
            return {nullptr, true};                             \
        }                                                       \
    }

#define TRY_TOKEN(type)                                         \
    {                                                           \
        auto [_, _try_error_] = consume_token_of_type(type);    \
        if(_try_error_) {                                       \
            return {nullptr, true};                             \
        }                                                       \
    }

#define WIN(value)                                              \
    {value, false}

#define FAIL(line, character, message)                           \
    report_error(line, character, message);                      \
    return {nullptr, true};
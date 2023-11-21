#pragma once

#include <iostream>

#define USE_ASSERTS

#ifdef USE_ASSERTS
#define ASSERT_MSG(expr, message)                                                                                      \
    if (!(expr))                                                                                                       \
    {                                                                                                                  \
        std::cerr << "ASSERT :: " << __FILE_NAME__ << " :: line " << __LINE__ << "\n  --> (" << std::string(#expr)     \
                  << ")\n  --> \"" << message << "\"";                                                                 \
        exit(1);                                                                                                       \
    }

#define ASSERT(expr)                                                                                                   \
    if (!(expr))                                                                                                       \
    {                                                                                                                  \
        std::cerr << "ASSERT :: " << __FILE_NAME__ << " :: line " << __LINE__ << "\n  --> (" << std::string(#expr)     \
                  << ")";                                                                                              \
        exit(1);                                                                                                       \
    }
#else
#define ASSERT_MSG(expr, message)
#define ASSERT(expr)
#endif

#define NOT_IMPLEMENTED()                                                                                              \
    std::cerr << "NOT IMPLEMENTED :: " << __FILE_NAME__ << " :: line " << __LINE__ << "\n";                            \
    exit(1);

#define UNREACHABLE()                                                                                                  \
    std::cerr << "UNREACHABLE was reached:: " << __FILE_NAME__ << " :: line " << __LINE__ << "\n";                     \
    exit(1);

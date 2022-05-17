#pragma once

#define ASSERT(condition, message) \
    if (!(condition)) {            \
        printf("%s \n", message);  \
        assert(0);                 \
    }

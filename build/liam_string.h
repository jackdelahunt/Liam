
#pragma once
#include <string.h>
#include <stdlib.h>

struct string {
    char* str;
    size_t size;
    
    string(const char* c_str) {
        int str_size = strlen(c_str) + 1;
        size = str_size;
        str = (char*)malloc(sizeof(char) * size);
        memcpy(str, c_str, size);
    }
    
    ~string() {
        free(str);
    }
};

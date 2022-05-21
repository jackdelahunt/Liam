#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include "arena.h"

#define GROW(ptr, size) \
    void* ret = reallocarray(ptr, sizeof(*ptr), size); \
    if(ret == NULL) {          \
        printf("Error in realloc");                    \
    } else { \
        ptr = ret; \
    }

typedef struct Slice {
    const char* start;
    size_t length;
} Slice;

typedef struct Str {
    char* buffer;
    size_t length;
    size_t capacity;
} Str;


Slice make_slice(const char* start, size_t length);
Slice make_slice_c_str(const char* start);
bool compare_slice(Slice* l, const char* c_str);
void print_slice(Slice* slice);
bool is_char_num(char c);
bool slice_to_int(Slice* slice, int* dest);

Str make_string(const char* c_str);
Str make_big_string(const char* c_str);
void append_string(Str* str, char c);
char char_at_string(Str* str, size_t index);
void concat_string(Str* dest, Str* src);
Str read_file(const char* path);
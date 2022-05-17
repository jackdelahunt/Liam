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

Slice make_slice(const char* start, size_t length) {
    return (Slice){start, length};
}

Slice make_slice_c_str(const char* start) {
    size_t length = strlen(start);
    return (Slice){start, length};
}

bool compare_slice(Slice* l, const char* c_str) {
    size_t length = strlen(c_str);

    if(l->length != length) return false;

    for(int i = 0; i < l->length; i++) {
        if(l->start[i] != c_str[i]) return false;
    }

    return true;
}

void print_slice(Slice* slice) {
    for(int i = 0; i < slice->length; i++) {
        printf("%c", slice->start[i]);
    }
    printf("\n");
}

bool is_char_num(char c) {
    // 48 is zero... 57 is 9
    return (int)c >= 48 && (int)c <= 57;
}

bool slice_to_int(Slice* slice, int* dest) {
    const char* current = slice->start;
    int factor = slice->length - 1;
    int final_value = 0;

    for(int i = 0; i < slice->length; i++) {

        if(is_char_num(*current)) {
            int converted = *current - 48; // remove 0 base number to map 0 -> 0 and so on
            int multiplier = (int)pow(10, factor);
            final_value += converted * multiplier;
        } else {return false; };

        factor--;
        current += 1;
    }

    *dest = final_value;
    return true;
}

typedef struct Str {
    char* buffer;
    size_t length;
    size_t capacity;
} Str;

Str make_string(const char* c_str) {
    size_t length = strlen(c_str);
    size_t capacity = (length * 2) + 10;
    char* buffer = ALLOC(capacity);
    strncpy(buffer, c_str, length);
    return (Str){buffer, length, capacity};
}

Str make_big_string(const char* c_str) {
    size_t length = strlen(c_str);
    size_t capacity = 2048 + length;
    char* buffer = ALLOC(capacity);
    strncpy(buffer, c_str, length);
    return (Str){buffer, length, capacity};
}

void append_string(Str* str, char c) {
    if(str->length >= str->capacity) {
        str->capacity *= 2;
        GROW(str->buffer, str->capacity);
    }

    str->buffer[str->length++] = c;
}

char char_at_string(Str* str, size_t index) {
    assert(str->length > index);
    return str->buffer[index];
}

void concat_string(Str* dest, Str* src) {
    if(dest->capacity - dest->length < src->length) {
        dest->capacity = (dest->capacity * 2) + src->length;
        GROW(dest->buffer, dest->capacity);
    }

    strcpy(&dest->buffer[dest->length], src->buffer);
    dest->length += src->length;
}

Str read_file(const char* path) {
    Str source = make_big_string("");
    FILE *file = fopen(path, "r");

    int c = fgetc(file);
    while (c != EOF) {
        append_string(&source, (char)c);
        c = fgetc(file);
    }

    fclose(file);
    return source;
}
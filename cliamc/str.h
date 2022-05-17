#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

bool slice_to_int(Slice* slice, int* dest) {
    // copy slice value into buffer and null terminate the string
    char buffer[128];
    memcpy(buffer, slice->start, slice->length);
    buffer[slice->length] = '\0';

    errno = 0;
    int value = strtol(buffer, NULL, 10); // NOTE: converting long to int here, might break something
    if(errno == ERANGE) return false;

    *dest = value;
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
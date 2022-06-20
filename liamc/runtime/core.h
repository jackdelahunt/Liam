#pragma once
#include <stdint.h>
#include <iostream>
#include <stdlib.h>

typedef uint8_t u8;
typedef int8_t s8;

typedef int64_t s64;
typedef int u64;
typedef double f64;

typedef int32_t s32;
typedef uint32_t u32;
typedef float f32;

template <typename T>
void print(T t) {
    std::cout << t;
}

template <typename T>
void println(T t) {
    std::cout << t << "\n";
}

template <typename T>
T* make(T t) {
    auto ptr = (T*)malloc(sizeof(T));
    *ptr = t;
    return ptr;
}

template <typename T>
void destroy(T* t) {
    free(t);
}


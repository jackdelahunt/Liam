#pragma once
#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include <sstream> 
#include <string>

typedef uint8_t u8;
typedef int8_t s8;

typedef int64_t s64;
typedef double f64;

typedef int32_t s32;
typedef uint32_t u32;
typedef float f32;

struct u64 {    
    uint64_t n;

    std::string pretty_string(std::string indentation) {
        return indentation + std::to_string(n);
    }
};

u64 make_u64(uint64_t n) {
    return u64{n};
}

struct string {
    char* chars;
    uint64_t length;

    std::string pretty_string(std::string indentation) {
        return indentation +  std::string(chars);
    }
};

string make_string(char* chars, uint64_t length) {
    return string{chars, length};
}

u64 len(string str) {
    return make_u64(str.length);
}

std::string pretty_string_pointer(std::string indentation, void *ptr) {
    std::stringstream ss;
    ss << ptr;  
    return indentation + ss.str(); 
}

template <typename T>
void print(T t) {
    std::cout << t.pretty_string("");
}

template <typename T>
void println(T t) {
    std::cout << t.pretty_string("") << "\n";
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


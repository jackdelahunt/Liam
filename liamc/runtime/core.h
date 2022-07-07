#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

typedef uint8_t u8;
typedef int8_t s8;

typedef uint64_t u64;
typedef int64_t s64;
typedef double f64;

typedef int32_t s32;
typedef uint32_t u32;
typedef float f32;

// used to make sure size of literal
u64 _u64(int n) {
    return n;
}

// used to make sure size of literal
s64 _s64(int n) {
    return n;
}

std::string pretty_string_builtin(std::string indentation, u64 n) {
    return std::to_string(n);
}

std::string pretty_string_builtin(std::string indentation, bool b) {
    return b ? "true" : "false";
}

struct str {
    char *chars;
    uint64_t length;

    std::string pretty_string(std::string indentation) {
        char *cpy = (char *)malloc(sizeof(char) * length);
        memcpy(cpy, chars, length);
        cpy[length] = '\0';
        auto print = indentation + std::string(cpy);
        free(cpy);
        return print;
    }

    friend bool operator==(const str &l, const str &r) {
        if (l.length != r.length)
            return false;

        for (uint64_t i = 0; i < l.length; i++)
        {
            if (l.chars[i] == r.chars[i])
            {}
            else
                return false;
        }

        return true;
    }
};

std::ostream& operator<<(std::ostream& os, const str &obj) {
    for(int i = 0; i < obj.length; i++) {
        os << obj.chars[i];
    }
    return os;
}

str make_str(char *chars, uint64_t length) {
    return str{chars, length};
}

u64 len(str s) {
    return s.length;
}

std::string pretty_string_pointer(std::string indentation, void *ptr) {
    std::stringstream ss;
    ss << ptr;
    return indentation + ss.str();
}

template <typename T> void print(T t) {
    std::cout << t;
}

template <typename T> void println(T t) {
    std::cout << t.pretty_string("") << "\n";
}

template <typename T> T *alloc(T t) {
    auto ptr = (T *)malloc(sizeof(T));
    *ptr = t;
    return ptr;
}

template <typename T> void destroy(T *t) {
    free(t);
}

template <typename T> u64 hashcode(T *t) {
    return u64{(uint64_t)t};
}

template <typename T> struct Array {
    u64 length;
    u64 capacity;
    std::vector<T> data;
};

template <typename T> Array<T> make_array() {
    u64 start_capacity = 10;
    return Array<T>{
        .length = 0,
        .capacity = start_capacity,
        .data = std::vector<T>(),
    };
}

template <typename T> void array_add(Array<T> *array, T t) {
    array->data.push_back(t);
}

template <typename T> T array_at(Array<T> *array, u64 index) {
    return array->data.at(index);
}

template <typename T> u64 array_size(Array<T> *array) {
    return array->data.size();
}

struct String {
    std::string string;

    std::string pretty_string(std::string indentation) {
        return indentation + string;
    }
};

String make_string() {
    return String{.string = std::string()};
}

String make_string_from(str s) {
    return String{.string = std::string(s.chars)};
}

str to_str(String *s) {
    return make_str((char *)s->string.c_str(), s->string.size());
}

void string_append(String *s, String *x) {
    s->string.append(x->string);
}

String read(str path) {
    std::ifstream input{path.chars};

    if (!input.is_open())
    {
        std::cerr << "Couldn't read file: " << path.chars << "\n";
        exit(1);
    }

    std::string s;
    input >> s;

    return String{.string = s};
}

str string_substring(String *string, u64 start, u64 length) {
    return make_str((char *)&string->string.c_str()[start], length);
}

u64 string_length(String *string) {
    return string->string.size();
}
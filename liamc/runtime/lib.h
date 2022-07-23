#pragma once
#include <stdlib.h>

#include <fstream>
#include <string>
#include <vector>

#include "core.cpp"
#include "core.h"

#include "memory.cpp"
#include "memory.h"

#include "builtin_string.cpp"
#include "builtin_string.h"

#include "io.h"

template <typename T> struct Array {
    u64 length;
    u64 capacity;
    std::vector<T> data;
};

template <typename T> Array<T> make_array() {
    u64 start_capacity = 10;
    return Array<T>{
        .length   = 0,
        .capacity = start_capacity,
        .data     = std::vector<T>(),
    };
}

template <typename T> void array_add(Array<T> *array, const T &t) {
    array->data.push_back(t);
}

template <typename T> T array_at(Array<T> *array, u64 index) {
    return array->data.at(index);
}

template <typename T> u64 array_size(Array<T> *array) {
    return array->data.size();
}
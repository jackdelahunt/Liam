#pragma once
#include "core.h"
#include "memory.h"
#include <map>

template <typename T> struct Array {
    T *data;
    int length;
    int capacity;
};

template <typename T> Array<T> make_array() {
    return Array<T>{.data = NULL, .length = 0, .capacity = 0};
}

template <typename T> void __append(Array<T> *self, T value) {
    if (self->length == self->capacity)
    {
        self->capacity *= 2;
        self->data = re_alloc<T>(self->data, self->capacity * sizeof(T));
    }
    self->data[self->length++] = value;
}

template <typename T> T __get(Array<T> *self, u64 index) {
    return self->data[index];
}

template <typename T> T* __get_ptr(Array<T> *self, u64 index) {
    return &self->data[index];
}

template <typename T> void __set(Array<T> *self, u64 index, T t) {
    self->data[index] = t;
}

template <typename T> void __insert(Array<T> *self, u64 index, T value) {
    if (self->length == self->capacity)
    {
        self->capacity *= 2;
        self->data = re_alloc<T>(self->data, self->capacity * sizeof(T));
    }
    for (int i = self->length; i > index; i--)
    { self->data[i] = self->data[i - 1]; }
    self->data[index] = value;
    self->length++;
}

template <typename T> void __remove(Array<T> *self, u64 index) {
    for (int i = index; i < self->length - 1; i++)
    { self->data[i] = self->data[i + 1]; }
    self->length--;
}

template <typename T> void __free(Array<T> *self) {
    destroy(self->data);
    self->capacity = 0;
    self->length   = 0;
}
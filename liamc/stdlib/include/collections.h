#pragma once
#include "core.h"
#include "memory.h"
#include <map>

template <typename T> struct Array {
    T *data;
    int length;
    int capacity;
};

template <typename K, typename V> struct Map { std::map<K, V> map; };

/*
 * extern fn make_map[K. V](): Map[K, V];
extern fn map_insert[K, V](k: K, v: V): void;
extern fn map_get[K, V](k: K): V;

 */

template <typename K, typename V> Map<K, V> make_map() {
    return Map<K, V>{.map = std::map<K, V>()};
}

template <typename K, typename V> void __insert(Map<K, V> *self, K k, V v) {
    self->map[k] = v;
}

template <typename K, typename V> V __get(Map<K, V> *self, K k) {
    return self->map[k];
}

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

template <typename T> T __index(Array<T> *self, int index) {
    return self->data[index];
}

template <typename T> void __insert(Array<T> *self, int index, T value) {
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

template <typename T> void __remove(Array<T> *self, int index) {
    for (int i = index; i < self->length - 1; i++)
    { self->data[i] = self->data[i + 1]; }
    self->length--;
}

template <typename T> void __free(Array<T> *self) {
    destroy(self->data);
    self->capacity = 0;
    self->length   = 0;
}
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

template <typename K, typename V> Map<K, V> map_insert(Map<K, V> *map, K k, V v) {
    (*map)[k] = v;
}

template <typename K, typename V> Map<K, V> map_get(Map<K, V> *map, K k) {
    return (*map)[k];
}

template <typename T> Array<T> make_array() {
    return Array<T>{.data = NULL, .length = 0, .capacity = 0};
}

template <typename T> void array_append(Array<T> *array, T value) {
    if (array->length == array->capacity)
    {
        array->capacity *= 2;
        array->data = re_alloc<T>(array->data, array->capacity * sizeof(T));
    }
    array->data[array->length++] = value;
}

template <typename T> T array_index(Array<T> *array, int index) {
    return array->data[index];
}

template <typename T> void array_insert(Array<T> *array, int index, T value) {
    if (array->length == array->capacity)
    {
        array->capacity *= 2;
        array->data = re_alloc<T>(array->data, array->capacity * sizeof(T));
    }
    for (int i = array->length; i > index; i--)
    { array->data[i] = array->data[i - 1]; }
    array->data[index] = value;
    array->length++;
}

template <typename T> void array_remove(Array<T> *array, int index) {
    for (int i = index; i < array->length - 1; i++)
    { array->data[i] = array->data[i + 1]; }
    array->length--;
}

template <typename T> void array_free(Array<T> *array) {
    destroy(array->data);
    array->capacity = 0;
    array->length   = 0;
}
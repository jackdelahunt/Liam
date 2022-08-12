#include "core.h"
#include "memory.h"

template <typename T> struct Array {
    T *data;
    int length;
    int capacity;
};

template <typename T> std::ostream &operator<<(std::ostream &os, const Array<T> &obj) {

    os << "[";

    for (u64 i = 0; i < obj.length; i++)
    {
        os << obj.data[i];

        if (i + 1 < obj.length)
        { os << ", "; }
    }

    os << "]";
    return os;
}

template <typename T> Array<T> make_array() {
    return Array<T>{.data = NULL, .length = 0, .capacity = 0};
}

template <typename T> void array_append(Array<T> *array, T value) {
    if (array->length == array->capacity)
    {
        array->capacity *= 2;
        array->data = (T *)Internal::allocator->re_alloc(array->data, array->capacity * sizeof(T));
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
        array->data = (T *)Internal::allocator->re_alloc(array->data, array->capacity * sizeof(T));
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
    Internal::allocator->destroy(array->data);
    array->capacity = 0;
    array->length   = 0;
}
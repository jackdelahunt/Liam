#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

namespace liam {
    typedef uint64_t Byte;

    template <typename T>
    struct Slice {
        T* start;
        size_t count;
    };

    template <typename T>
    struct Array {
        T* start;
        size_t count;
        size_t capacity;

        Array() {
            this->count = 0;
            this->capacity = 10;
            this->start = (T*)malloc(sizeof(T) * this->capacity);
        }

        Array(size_t capacity) {
            this->count = 0;
            this->capacity = capacity;
            this->start = (T*)malloc(sizeof(T) * capacity);
        }

        void add(T value) {
            maybe_expand();
            count++;
            start[count - 1] = value;
        }

        void maybe_expand() {
            if(count >= capacity) {
                capacity *= 2;
                start = (T*)realloc(start, capacity);
            }
        }

        void print() {
            if(count > 0) {
                for(int i = 0; i < count; i++) {
                    printf("%d", start[i]);
                }
                printf("\n");
            }
        }
    };
}
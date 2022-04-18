#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

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

enum OpCode : Byte {
    ADD     = 0,
    PRINT   = 1,
    PUSH    = 2,
    GOTO    = 3,
};

struct VM
{
    Byte byte_code[1024]    = {0};
    Byte stack[1024]        = {0};
    Byte stack_ptr          = 0; // one more then last valid value
    Byte memory_ptr         = 0; // one more then last valid byte
    Byte instruction_ptr    = 0; // current valid instruction

    void push_byte(Byte byte);
    void run();
    void add_op();
    void print_op();
    void push_op();
    void goto_op();
    Byte next_byte();
    void stack_push(Byte byte);
    Byte stack_pop();
};

void panic(const std::string& msg);
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

        void push_byte(Byte byte) {
            byte_code[memory_ptr] = byte;
            memory_ptr++;
        }

        void run() {
            while(instruction_ptr < memory_ptr) {
                switch (byte_code[instruction_ptr])
                {
                    case OpCode::ADD: add_op();     instruction_ptr++;  break;
                    case OpCode::PRINT: print_op(); instruction_ptr++;  break;
                    case OpCode::PUSH: push_op();   instruction_ptr++;  break;
                    case OpCode::GOTO: goto_op();                       break;
                }
            }
        }

        void add_op() {
            auto l = stack_pop();
            auto r = stack_pop();
            stack_push(l + r);
        }

        void print_op() {
            printf("%lu\n", stack[stack_ptr - 1]);
        }

        void push_op() {
            stack_push(next_byte());
        }

        void goto_op() {
            Byte to = next_byte();
            instruction_ptr = to;
        }

        Byte next_byte() {
            return byte_code[++instruction_ptr];
        }

        void stack_push(Byte byte) {
            stack[stack_ptr] = byte;
            stack_ptr++;
        }

        Byte stack_pop() {
            return stack[--stack_ptr];
        }
    };

    void panic(const std::string& msg)
    {
        std::cerr << msg << std::endl;
        exit(1);
    }
}
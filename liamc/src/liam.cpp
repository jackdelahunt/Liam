#pragma once

#include "liam.h"
#include <iostream>

void VM::push_byte(Byte byte) {
    byte_code[memory_ptr] = byte;
    memory_ptr++;
}

void VM::run() {
    while (instruction_ptr < memory_ptr) {
        switch (byte_code[instruction_ptr])
        {
        case OpCode::ADD: add_op();     instruction_ptr++;  break;
        case OpCode::PRINT: print_op(); instruction_ptr++;  break;
        case OpCode::PUSH: push_op();   instruction_ptr++;  break;
        case OpCode::GOTO: goto_op();                       break;
        }
    }
}

void VM::add_op() {
    auto l = stack_pop();
    auto r = stack_pop();
    stack_push(l + r);
}

void VM::print_op() {
    printf("%lu\n", stack[stack_ptr - 1]);
}

void VM::push_op() {
    stack_push(next_byte());
}

void VM::goto_op() {
    Byte to = next_byte();
    instruction_ptr = to;
}

Byte VM::next_byte() {
    return byte_code[++instruction_ptr];
}

void VM::stack_push(Byte byte) {
    stack[stack_ptr] = byte;
    stack_ptr++;
}

Byte VM::stack_pop() {
    return stack[--stack_ptr];
}

void panic(const std::string& msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}
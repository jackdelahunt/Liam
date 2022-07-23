#pragma once

#include "builtin_string.h"
#include "core.h"
#include <fstream>
#include <sstream>
#include <string>

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

std::string pretty_string_builtin(std::string indentation, u64 n) {
    return std::to_string(n);
}

std::string pretty_string_builtin(std::string indentation, bool b) {
    return b ? "true" : "false";
}

std::string pretty_string_pointer(std::string indentation, void *ptr) {
    std::stringstream ss;
    ss << ptr;
    return indentation + ss.str();
}

template <typename T> void print(const T &t) {
    std::cout << t;
}

template <typename T> void println(const T &t) {
    std::cout << t << "\n";
}
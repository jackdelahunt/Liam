#pragma once

#include <string>

#include "cxxopts.h"
#include "liam.h"

struct Arguments;
extern Arguments *args;

struct Arguments {
    std::string out_path;
    std::string in_path;
    bool codegen;
    bool time;
    std::string stdlib;
    bool help;
    std::string include;
    std::string emit;
    bool debug;

    cxxopts::Options *options;
    cxxopts::ParseResult result;

    static void New(int argc, char **argv);

    template <typename T> T value(std::string option) {
        return this->result[option].as<T>();
    }
};

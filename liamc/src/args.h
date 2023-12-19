#pragma once

#include <cxxopts/cxxopts.h>
#include <string>

#include "liam.h"

struct Arguments;
extern Arguments *args;

struct Arguments {
    std::string              out_path;
    bool                     emit;
    bool                     time;
    std::string              include;
    bool                     test;
    std::vector<std::string> files;

    cxxopts::Options    *options;
    cxxopts::ParseResult result;

    static void make(int argc, char **argv);

    template <typename T> T value(std::string option) {
        return this->result[option].as<T>();
    }
};

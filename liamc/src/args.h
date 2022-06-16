#pragma once

#include <string>
#include "cxxopts.h"
#include "liam.h"

struct Arguments;
Arguments* args = NULL;

struct Arguments {
    
    std::string out_path;
    cxxopts::Options* options;
    cxxopts::ParseResult result;

    static void New(int argc, char** argv) {
        args = new Arguments{};
        auto options = new cxxopts::Options("liamc", "Liam programming language compiler");
        options->add_options()("o,out", "Output filen path", cxxopts::value<std::string>()->default_value("out.cpp")); 
        args->result = options->parse(argc, argv);
        args->options = options;
    }

    template <typename T>
    T value(std::string option) {
        return  this->result[option].as<T>();
    }
};



#pragma once

#include <string>

#include "cxxopts.h"
#include "liam.h"

struct Arguments;
Arguments *args = NULL;

struct Arguments
{
    std::string out_path;
    cxxopts::Options *options;
    cxxopts::ParseResult result;

    static void New(int argc, char **argv)
    {
        args = new Arguments{};
        auto options = new cxxopts::Options("liamc", "Liam programming language compiler");
        options->add_options()("o,out", "Output file path", cxxopts::value<std::string>()->default_value("out.cpp"));
        options->add_options()("i,in", "Input file path", cxxopts::value<std::string>());
        options->add_options()("c,codegen", "Print codegen to stdout instead of writing to a file",
                               cxxopts::value<bool>()->default_value("false"));
        options->add_options()("t,time", "Print times", cxxopts::value<bool>()->default_value("false"));
        args->result = options->parse(argc, argv);
        args->options = options;

        if (!args->result.hasValue<std::string>("in"))
        {
            panic("No in flag given");
        }
    }

    template <typename T> T value(std::string option)
    {
        return this->result[option].as<T>();
    }
};

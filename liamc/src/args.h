#pragma once

#include <string>

#include "cxxopts.h"
#include "liam.h"

struct Arguments;
Arguments *args = NULL;

struct Arguments {
    std::string out_path;
    std::string in_path;
    bool codegen;
    bool time;
    bool runtime;
    bool help;
    std::string include;


    cxxopts::Options *options;
    cxxopts::ParseResult result;

    static void New(int argc, char **argv) {
        args         = new Arguments{};
        auto options = new cxxopts::Options("liamc", "Liam programming language compiler");

        // required fields
        options->add_options()("i,in", "Input file path", cxxopts::value<std::string>());

        // optionals with defaults
        options->add_options()("o,out", "Output file path", cxxopts::value<std::string>()->default_value("out.cpp"));
        options->add_options()("c,codegen", "Print codegen to stdout", cxxopts::value<bool>()->default_value("false"));
        options->add_options()("t,time", "Print times", cxxopts::value<bool>()->default_value("false"));
        options->add_options()("r,runtime", "Get the runtime location", cxxopts::value<bool>()->default_value("false"));
        options->add_options()("h,help", "See this help screen", cxxopts::value<bool>()->default_value("false"));
        options->add_options()("I,include", "Include path", cxxopts::value<std::string>()->default_value("runtime/"));

        args->result  = options->parse(argc, argv);
        args->options = options;

        // optional
        args->out_path = args->value<std::string>("out");
        args->codegen  = args->value<bool>("codegen");
        args->time     = args->value<bool>("time");
        args->runtime  = args->value<bool>("runtime");
        args->help     = args->value<bool>("help");
        args->include = args->value<std::string>("include");

        // required args
        if (args->result.hasValue<std::string>("in"))
        { args->in_path = args->value<std::string>("in"); }
        else
        {
            // if runtime or help is set then dont bother check for --in
            if (!(args->runtime || args->help))
                panic("No in flag given");
        }
    }

    template <typename T> T value(std::string option) {
        return this->result[option].as<T>();
    }
};

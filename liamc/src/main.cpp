#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "backends/cpp_backend.h"
#include "compiler.h"
#include "fmt/core.h"
#include "liam.h"

#ifndef TEST

void build_step(std::string *code);

s32 main(s32 argc, char **argv) {
    Arguments::New(argc, argv);

    if (args->help)
    {
        print_help();
        return 0;
    }

    TIME_START(l_p_time);
    auto files = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&typed_file);
    TIME_END(code_gen, "Code generation time");

    if (args->value<bool>("codegen"))
    { std::cout << code; }
    else
    { build_step(&code); }

    return 0;
}

void build_step(std::string *code) {

    auto unwanted_warnings = "-Wno-tautological-compare";

    // write cpp file
    std::ofstream out_file("out.cpp");
    out_file << *code;
    out_file.close();

    {
        FILE *file = popen(
            fmt::format("clang++ {} -I {} out.cpp -std=c++20 -o {}", unwanted_warnings, args->include, args->out_path)
                .c_str(),
            "r"
        );
        pclose(file);
    }
    {
        if (!args->emit)
        {
            FILE *file = popen("rm out.cpp", "r");
            pclose(file);
        }
    }
}
#endif
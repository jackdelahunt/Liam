#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "backends/cpp_backend.h"
#include "compiler.h"
#include "liam.h"
#include "fmt/core.h"

#ifndef TEST

void build_step(std::string *code);

s32 main(s32 argc, char **argv) {
    Arguments::New(argc, argv);

    if (args->runtime)
    {
        print_runtime();
        return 0;
    }

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
    {
        build_step(&code);
    }

    return 0;
}

void build_step(std::string *code) {

    // write cpp file
    std::ofstream out_file("out.cpp");
    out_file << *code;
    out_file.close();

    {
        FILE *file = popen(fmt::format("clang++ -I {} out.cpp -std=c++20 -o {}", args->include, args->out_path).c_str(), "r");
        pclose(file);
    }
    {
        FILE *file = popen("rm out.cpp", "r");
        pclose(file);
    }
}
#endif
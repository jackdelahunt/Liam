#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "compiler.h"
#include "cpp_backend.h"
#include "fmt/core.h"
#include "liam.h"
#include "llvm_backend.h"

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

    if (args->cpp)
    {
        auto code = CppBackend().emit(&typed_file);
        if (args->emit)
        { std::cout << code << "\n"; }

        build_step(&code);
    }
    else if (args->llvm)
    { LLVMBackend().emit(&typed_file); }

    TIME_END(code_gen, "Code generation time");

    return 0;
}

void build_step(std::string *code) {

    auto unwanted_warnings = "-Wno-tautological-compare -Wno-return-type";

    std::ofstream out_file;
    std::string out_file_path = "";

    if (args->cpp_source.empty())
    { out_file_path = "out.cpp"; }
    else
    { out_file_path = args->cpp_source; }

    out_file = std::ofstream(out_file_path);

    out_file << *code;
    out_file.close();

    auto command = fmt::format(
        "clang++ {} -I {} {} -std=c++20 -o {} ", unwanted_warnings, args->include, out_file_path, args->out_path
    );

    if (args->debug)
    { command += "-g "; }

    {
        FILE *file = popen(command.c_str(), "r");
        pclose(file);
    }
    {
        if (args->cpp_source.empty())
        {
            FILE *file = popen(fmt::format("rm {}", out_file_path).c_str(), "r");
            pclose(file);
        }
    }
}
#endif
#include <iostream>
#include <chrono>
#include <filesystem>
#include "liam.h"
#include "backends/cpp_backend.h"
#include "compiler.h"
#include "args.h"

s32 main(s32 argc, char** argv) {

    if (argc < 2) {
        panic("Not enough arguments: liamc <path>");
    }

    Arguments::New(argc, argv);
    std::cout << args->value<std::string>("out") << "\n";

    auto source = absolute(std::filesystem::path(argv[1]));

    TIME_START(l_p_time);
    auto files = lex_parse(source.string());
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&typed_file);
    std::ofstream out_file(args->value<std::string>("out"));
    out_file << code;
    out_file.close();
    TIME_END(code_gen, "Code generation time");


    return 0;
}
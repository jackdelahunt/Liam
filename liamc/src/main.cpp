#include <iostream>
#include <chrono>
#include <filesystem>
#include "liam.h"
#include "backends/cpp_backend.h"
#include "compiler.h"
#include "args.h"

s32 main(s32 argc, char** argv) {

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    auto files = lex_parse(args->value<std::string>("in"));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&typed_file);
    if(args->value<bool>("codegen")) {
        std::cout << code;
    } else {
        std::ofstream out_file(args->value<std::string>("out"));
        out_file << code;
        out_file.close();
    }
    TIME_END(code_gen, "Code generation time");


    return 0;
}
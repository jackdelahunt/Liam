#include <iostream>
#include <chrono>
#include <filesystem>
#include "liam.h"
#include "backends/cpp_backend.h"
#include "compiler.h"

s32 main(s32 argc, char** argv) {

#ifdef DEBUG
    auto source = std::filesystem::path("E:/JacksDocuments/Projects/Liam/liamc/Code/Code/main.liam");
#else
    if (argc < 2) {
        panic("Not enough arguments: liamc <path>");
    }

    auto source = absolute(std::filesystem::path(argv[1]));
#endif

    TIME_START(l_p_time);
    auto files = lex_parse(source.string());
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&typed_file);
    auto out_path = source.parent_path().string() + "/out.cpp";
    std::ofstream out_file(out_path);
    out_file << code;
    out_file.close();
    TIME_END(code_gen, "Code generation time");


    return 0;
}
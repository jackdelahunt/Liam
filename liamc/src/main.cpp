#include <iostream>
#include <chrono>
#include <filesystem>
#include "liam.h"
#include "backends/rust_backend.h"
#include "backends/cpp_backend.h"
#include "compiler.h"

s32 main(s32 argc, char** argv) {

    if (argc < 3) {
        panic("Not enough arguments: liamc <target> <path>");
    }
    auto target = std::string(argv[1]);
    auto source = absolute(std::filesystem::path(argv[2]));

    TIME_START(l_p_time);
    auto files = lex_parse(source.string());
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    if(target == "rust") {
        auto code = RustBackend().emit(&typed_file);
        auto out_path = source.parent_path().string() + "/out.rs";
        std::ofstream out_file(out_path);
        out_file << code;
        out_file.close();
    } else if(target == "cpp") {
        auto code = CBackend().emit(&typed_file);
        auto out_path = source.parent_path().string() + "/out.cpp";
        std::ofstream out_file(out_path);
        out_file << code;
        out_file.close();
    } else {
        panic("Unknown target! " + target);
    }
    TIME_END(code_gen, "Code generation time");


    return 0;
}
#include <iostream>
#include <chrono>
#include <filesystem>
#include "liam.h"
#include "rust_backend/rust_backend.h"
#include "compiler.h"
#include "macros.h"

int main(int argc, char** argv) {

    if (argc < 2) {
        panic("Not enough arguments");
    }
    auto source = absolute(std::filesystem::path(argv[1]));

    TIME_START(l_p_time);
    auto files = lex_parse(source.string());
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = RustBackend().emit(&typed_file);
    TIME_END(code_gen, "Code generation time");

    auto out_path = source.parent_path().string() + "/out.rs";
    std::ofstream out_file(out_path);
    out_file << code;
    out_file.close();

    return 0;
}
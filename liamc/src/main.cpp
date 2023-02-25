#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "backends/cpp_backend.h"
#include "compiler.h"
#include "liam.h"
#include <functional>

#ifndef TEST

i32 main(i32 argc, char **argv) {

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    auto files = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    auto typed_file = type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&typed_file);
    TIME_END(code_gen, "Code generation time");

    if (args->emit)
    { std::cout << code << "\n"; }

    std::ofstream out_file;
    out_file = std::ofstream(args->out_path);

    out_file << code;
    out_file.close();

    return 0;
}
#endif
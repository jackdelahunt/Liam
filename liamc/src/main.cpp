#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "cpp_backend.h"
#include "compiler.h"
#include "liam.h"
#include <functional>

i32 main(i32 argc, char **argv) {

    TIME_START(total_time);

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    auto files = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(borrow_checking_time);
    borrow_check(&files);
    TIME_END(borrow_checking_time, "Borrow checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&files);
    TIME_END(code_gen, "Code generation time");

    if (args->emit)
    { std::cout << code << "\n"; }

    std::ofstream out_file;
    out_file = std::ofstream(args->out_path);

    out_file << code;
    out_file.close();

    TIME_END(total_time, "Total compile time");

    return 0;
}
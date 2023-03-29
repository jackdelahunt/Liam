#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>

#include "args.h"
#include "compiler.h"
#include "file.h"
#include "liam.h"

i32 main(i32 argc, char **argv) {

    TIME_START(total_time);

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    auto modules = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    type_check(&modules);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen_time);
    auto code = code_gen(&modules);
    TIME_END(code_gen_time, "Code generation time");

    if (args->emit)
    { std::cout << code << "\n"; }

    std::ofstream out_file;
    out_file = std::ofstream(args->out_path);

    out_file << code;
    out_file.close();

    TIME_END(total_time, "Total compile time");

    if (args->time)
    {
        u64 total_line_count = 0;
        for (auto file_name_to_file_data : *FileManager::get_files())
        { total_line_count += file_name_to_file_data.second.line_count; }

        u64 total_time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             std::chrono::high_resolution_clock::now() - total_time
        )
                                             .count();

        std::cout << "Total line count :: " << total_line_count
                  << " :: lines/S :: " << (f64)total_line_count / ((f64)total_time_in_milliseconds / 1000.0) << "\n";
    }

    return 0;
}

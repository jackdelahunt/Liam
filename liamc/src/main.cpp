#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>

#include "args.h"
#include "cpp_backend.h"
#include "errors.h"
#include "file.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

CompilationUnit *lex_parse(std::filesystem::path file_path);
void type_check(CompilationUnit *file);
std::string code_gen(CompilationUnit *file);

i32 main(i32 argc, char **argv) {
    TIME_START(total_time);

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    CompilationUnit *file = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    type_check(file);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen_time);
    auto code = code_gen(file);
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
                  << " :: LOC/s :: " << (f64)total_line_count / ((f64)total_time_in_milliseconds / 1000.0) << "\n";
    }

    return 0;
}

CompilationUnit *lex_parse(std::filesystem::path file_path) {
    FileData *file_data               = FileManager::load(file_path.string());
    Lexer lexer                       = Lexer(file_data);
    CompilationUnit *compilation_unit = lexer.lex();
    Parser parser                     = Parser(compilation_unit);
    parser.parse();

    if (ErrorReporter::has_parse_errors())
    {
        for (auto &error : ErrorReporter::singleton->parse_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" + std::to_string(ErrorReporter::singleton->parse_errors.size()) +
            ")"
        );
    }

    return parser.compilation_unit;
}

void type_check(CompilationUnit *file) {

    TypeChecker().type_check(file);

    if (ErrorReporter::has_type_check_errors())
    {
        for (auto &error : ErrorReporter::singleton->type_check_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" +
            std::to_string(ErrorReporter::singleton->type_check_errors.size()) + ")"
        );
    }
}

std::string code_gen(CompilationUnit *file) {
    return CppBackend().emit(file);
}

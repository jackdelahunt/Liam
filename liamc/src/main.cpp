#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <vector>

#include "args.h"
#include "compilation_unit.h"
#include "cpp_backend.h"
#include "errors.h"
#include "file.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

CompilationBundle lex_parse();
void type_check(CompilationBundle *file);
std::string code_gen(CompilationBundle *file);

i32 main(i32 argc, char **argv) {
    TIME_START(total_time);

    Arguments::make(argc, argv);

    std::string in = args->files[0];

    TIME_START(lex_parse_time);
    CompilationBundle bundle = lex_parse();
    TIME_END(lex_parse_time, "Lex and parsing time");

    TIME_START(type_time);
    type_check(&bundle);
    TIME_END(type_time, "Type checking time");

    TIME_START(code_gen_time);
    auto code = code_gen(&bundle);
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
        for (auto &file_data : *FileManager::get_files())
        { total_line_count += file_data->line_count; }

        u64 total_time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             std::chrono::high_resolution_clock::now() - total_time
        )
                                             .count();

        std::cout << "Total line count :: " << total_line_count
                  << " :: LOC/s :: " << (f64)total_line_count / ((f64)total_time_in_milliseconds / 1000.0) << "\n";
    }

    return 0;
}

CompilationBundle lex_parse() {

    std::vector<CompilationUnit *> compilation_units;

    for (auto &input_file : args->files)
    {
        std::filesystem::path file_path   = std::filesystem::path(input_file);
        FileData *file_data               = FileManager::load_relative_from_cwd(file_path).value();
        Lexer lexer                       = Lexer(file_data);
        CompilationUnit *compilation_unit = lexer.lex();
        Parser parser                     = Parser(compilation_unit);
        parser.parse();
        compilation_units.push_back(parser.compilation_unit);
    }

    if (ErrorReporter::has_parse_errors())
    {
        for (auto &error : ErrorReporter::singleton->parse_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" + std::to_string(ErrorReporter::singleton->parse_errors.size()) +
            ")"
        );
    }

    return CompilationBundle(compilation_units);
}

void type_check(CompilationBundle *bundle) {
    TypeChecker type_checker = TypeChecker();
    type_checker.type_check(bundle);

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

std::string code_gen(CompilationBundle *bundle) {
    std::string source;
    for (CompilationUnit *compilation_unit : bundle->compilation_units)
    { source.append(CppBackend().emit(compilation_unit)); }
    return source;
}

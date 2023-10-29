#include "compiler.h"
#include <set>
#include <format>

CompilationUnit *lex_parse(std::filesystem::path file_path) {
    FileData *file_data               = FileManager::load(file_path.string());
    Lexer lexer                       = Lexer(file_data);
    CompilationUnit *compilation_unit = lexer.lex();
    Parser parser                     = Parser(compilation_unit);
    parser.parse();

    i64 i = 0;
    for(auto& token_data : compilation_unit->token_buffer) {
        auto s = std::format(">{:4} :: {:12} ({} : {})\n", i, get_token_type_string(token_data.token_type), token_data.span.start, token_data.span.end);
        std::cout << s;
        i++;
    }

    if (ErrorReporter::has_parse_errors())
    {
        for (auto &error : ErrorReporter::singleton->parse_errors)
        {
            error.print_error_message();
        }

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
        {
            error.print_error_message();
        }

        panic(
            "Cannot continue with errors :: count (" +
            std::to_string(ErrorReporter::singleton->type_check_errors.size()) + ")"
        );
    }
}

std::string code_gen(CompilationUnit *file) {
    return CppBackend().emit(file);
}

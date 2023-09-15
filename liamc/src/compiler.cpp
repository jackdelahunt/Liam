#include "compiler.h"
#include <set>

File * lex_parse(std::filesystem::path file_path) {

    Lexer lexer = Lexer(file_path);
    lexer.lex();
    Parser parser = Parser(file_path, &lexer.tokens);
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

    return parser.file;
}

void type_check(File *file) {

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

std::string code_gen(File *file) {
    return CppBackend().emit(file);
}

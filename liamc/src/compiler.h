#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

std::vector<File *> lex_parse(std::filesystem::path path) {

    std::vector<std::string> import_path_queue = {path.string()};
    auto files                                 = std::vector<File *>();
    auto imported_paths                        = std::unordered_map<std::string, int>();

    while (import_path_queue.size() > 0)
    {
        std::string current_import_path = import_path_queue.at(import_path_queue.size() - 1);
        import_path_queue.pop_back();

        auto lexer = Lexer(current_import_path);
        lexer.lex();
        auto parser = Parser(current_import_path, lexer.tokens);
        parser.parse();

        files.emplace_back(parser.file);

        for (auto import : parser.file->imports)
        {
            if (!imported_paths.contains(import))
            {
                imported_paths[import] = 420;
                import_path_queue.push_back(import);
            }
        }
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

    return files;
}

void type_check(std::vector<File *> *files) {
    TypeChecker().type_check(files);

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

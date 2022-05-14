#pragma once
#include <string>
#include <vector>
#include "parser.h"
#include "type_checker.h"
#include "lexer.h"
#include "errors.h"
#include <filesystem>

std::vector<File> lex_parse(std::filesystem::path path) {
    auto files = std::vector<File>();

    auto lexer = Lexer(path);
    lexer.lex();
    auto parser = Parser(path, lexer.tokens);
    auto file = parser.parse();

    files.emplace_back(file);

    for(auto& import_path : file.imports) {
        auto imported = lex_parse(import_path);
        files.insert(files.end(), imported.begin(), imported.end());
    }

    if(ErrorReporter::has_errors()) {
        for(auto error : *ErrorReporter::all_reports()) {
            std::cout << error.file << ":" << error.line << ":" << error.character << " :: " << error.error << "\n";
        }

        panic("Cannot continue with errors :: count (" + std::to_string(ErrorReporter::all_reports()->size()) + ")");
    }

    return files;
}

inline File type_check(std::vector<File>* files) {
    auto tc = TypeChecker();
    return TypeChecker().type_check(files);
}



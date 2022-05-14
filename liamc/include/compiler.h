#pragma once
#include <string>
#include <vector>
#include "parser.h"
#include "type_checker.h"
#include "lexer.h"
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

    return files;
}

inline TypedFile type_check(std::vector<File>* files) {
    auto tc = TypeChecker();
    return TypeChecker().type_check(files);
}



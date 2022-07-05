#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

std::vector<File> lex_parse(std::filesystem::path path, std::vector<std::string> *imports = NULL) {
    auto files = std::vector<File>();

    auto lexer = Lexer(path);
    lexer.lex();
    auto parser = Parser(path, lexer.tokens);
    auto file = parser.parse();

    files.emplace_back(file);

    if (imports == NULL)
    { // if this is the top level call
        imports = new std::vector<std::string>();
    }

    imports->emplace_back(path.string());

    for (auto &import_path : file.imports)
    {
        bool already_declared = false;
        for (u64 i = 0; i < imports->size(); i++)
        {
            if (import_path == imports->at(i))
            {
                already_declared = true;
                continue;
            }
        }

        if (already_declared)
            continue;

        auto imported = lex_parse(import_path, imports);
        files.insert(files.end(), imported.begin(), imported.end());
    }

    if (ErrorReporter::has_errors())
    {
        for (auto error : *ErrorReporter::all_reports())
        { std::cout << error.file << ":" << error.line << ":" << error.character << " :: " << error.error << "\n"; }

        panic("Cannot continue with errors :: count (" + std::to_string(ErrorReporter::all_reports()->size()) + ")");
    }

    return files;
}

inline File type_check(std::vector<File> *files) {
    auto tc = TypeChecker();
    return TypeChecker().type_check(files);
}

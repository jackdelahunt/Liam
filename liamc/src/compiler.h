#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

std::vector<File *> lex_parse(std::filesystem::path path, std::vector<std::string> *imports = NULL) {

    auto files = std::vector<File *>();

    auto lexer = Lexer(path);
    lexer.lex();
    auto parser = Parser(path, lexer.tokens);
    parser.parse();

    files.emplace_back(parser.file);

    if (imports == NULL)
    { // if this is the top level call
        imports = new std::vector<std::string>();
    }

    imports->emplace_back(path.string());

    for (auto &import_path : parser.file->imports)
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

File type_check(std::vector<File *> *files) {
    auto file = TypeChecker().type_check(files);

    if (ErrorReporter::has_type_check_errors())
    {
        for (auto &error : ErrorReporter::singleton->type_check_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" +
            std::to_string(ErrorReporter::singleton->type_check_errors.size()) + ")"
        );
    }

    return file;
}

void print_runtime() {
#ifdef WINDOWS
    panic("This flag is not supported on windows yet...");
#else
    auto path = std::filesystem::canonical("/proc/self/exe").parent_path().string() + "/runtime";
    std::cout << path;
#endif
}

void print_help() {
    std::cout << args->options->help();
}

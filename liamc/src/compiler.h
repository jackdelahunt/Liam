#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_backend.h"
#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "module.h"
#include "parser.h"
#include "type_checker.h"

Module *create_lex_and_parse_module_from_path(std::string module_name, std::filesystem::path module_path) {
    auto files = std::vector<File *>();

    for (auto &direcotry_item : std::filesystem::directory_iterator(module_path))
    {
        if (!std::filesystem::is_regular_file(direcotry_item))
            continue;

        auto file_path = direcotry_item.path();

        if (!file_path.has_extension())
            continue;
        if (!(file_path.extension().string() == ".liam"))
            continue;

        Lexer lexer = Lexer(file_path);
        lexer.lex();

        Parser parser = Parser(file_path, &lexer.tokens);
        parser.parse();

        files.push_back(parser.file);
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

    return new Module(module_name, module_path, files);
}

std::vector<Module *> lex_parse(std::filesystem::path starting_import_path) {
    bool is_root                                                = true;
    std::vector<std::filesystem::path> module_import_path_queue = {
        std::filesystem::path(starting_import_path.parent_path())};
    auto modules                       = std::vector<Module *>();
    auto already_imported_module_paths = std::unordered_map<std::string, int>();

    while (module_import_path_queue.size() > 0)
    {
        auto current_module_path = module_import_path_queue.at(module_import_path_queue.size() - 1);
        module_import_path_queue.pop_back();

        std::string module_name;
        if (is_root)
        {
            module_name = "root";
            is_root     = false;
        }
        else
        { module_name = current_module_path.stem(); }

        auto created_module = create_lex_and_parse_module_from_path(module_name, current_module_path);
        modules.push_back(created_module);

        for (auto &file : created_module->files)
        {
            for (auto import_stmt : file->top_level_import_statements)
            {
                auto import_path                          = import_stmt->path->token.string;
                auto module_path_and_relative_import_path = created_module->path.append(import_path);
                auto absolute_path        = std::filesystem::absolute(module_path_and_relative_import_path);
                auto absolute_path_string = absolute_path.string();

                if (!(already_imported_module_paths.contains(absolute_path_string)))
                {
                    already_imported_module_paths[absolute_path_string] = 420;
                    module_import_path_queue.push_back(absolute_path);
                }
            }
        }
    }

    return modules;
}

void type_check(std::vector<Module *> *modules) {

    TypeChecker().type_check(modules);

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

std::string code_gen(std::vector<Module *> *modules) {

    ASSERT(modules->size() > 0);

    auto root_module = modules->at(0);
    return CppBackend().emit(&root_module->files);
}

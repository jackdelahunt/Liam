#pragma once

#include <filesystem>
#include <utility>

#include "ast.h"
#include "file.h"

struct CompilationUnit {
    std::filesystem::path path;
    FileData *file_data;
    std::vector<Token> tokens;
    std::vector<ImportStatement *> top_level_import_statements;
    std::vector<StructStatement *> top_level_struct_statements;
    std::vector<FnStatement *> top_level_fn_statements;
    std::vector<EnumStatement *> top_level_enum_statements;

    CompilationUnit(FileData *file_data, std::vector<Token> tokens) {
        this->file_data                   = file_data;
        this->tokens                      = std::move(tokens);
        this->top_level_import_statements = std::vector<ImportStatement *>();
        this->top_level_struct_statements = std::vector<StructStatement *>();
        this->top_level_fn_statements     = std::vector<FnStatement *>();
        this->top_level_enum_statements   = std::vector<EnumStatement *>();
    }
};
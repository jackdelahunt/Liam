#pragma once

#include <filesystem>

#include "file.h"
#include "ast.h"

struct CompilationUnit {
    std::filesystem::path path;
    FileData *file_data;

    std::vector<ImportStatement *> top_level_import_statements;
    std::vector<StructStatement *> top_level_struct_statements;
    std::vector<FnStatement *> top_level_fn_statements;
    std::vector<EnumStatement *> top_level_enum_statements;

    CompilationUnit(FileData *file_data) {
        this->file_data = file_data;

        this->top_level_import_statements = std::vector<ImportStatement *>();
        this->top_level_struct_statements = std::vector<StructStatement *>();
        this->top_level_fn_statements     = std::vector<FnStatement *>();
        this->top_level_enum_statements   = std::vector<EnumStatement *>();
    }
};
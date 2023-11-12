#pragma once

#include <filesystem>
#include <utility>

#include "ast.h"
#include "file.h"

struct FileData;

struct CompilationUnit {
    FileData *file_data;
    std::vector<Token> token_buffer;
    std::vector<StructStatement *> top_level_struct_statements;
    std::vector<FnStatement *> top_level_fn_statements;

    CompilationUnit(FileData *file_data, std::vector<Token> token_buffer);
    Token *get_token(TokenIndex token_index);
    std::string get_token_string_from_index(TokenIndex token_index);
};

struct CompilationBundle {
    std::vector<CompilationUnit *> compilation_units;

    CompilationBundle(std::vector<CompilationUnit *> compilation_units);
};

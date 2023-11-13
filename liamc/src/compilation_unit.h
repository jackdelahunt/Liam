#pragma once

#include <filesystem>
#include <utility>
#include <unordered_map>

#include "ast.h"
#include "file.h"

struct FileData;

typedef std::unordered_map<std::string, TypeInfo *> Scope;

struct CompilationUnit {
    FileData *file_data;
    std::vector<Token> token_buffer;
    std::vector<StructStatement *> top_level_struct_statements;
    std::vector<FnStatement *> top_level_fn_statements;
    std::vector<ImportStatement *> top_level_import_statements;
   
    Scope global_namespace_scope;
    Scope global_type_scope;
    Scope global_fn_scope;
    
    CompilationUnit(FileData *file_data, std::vector<Token> token_buffer);
    
    Token *get_token(TokenIndex token_index);
    std::string get_token_string_from_index(TokenIndex token_index);
    void add_type_to_scope(TokenIndex token_index, TypeInfo *type_info);
    void add_fn_to_scope(TokenIndex token_index, TypeInfo *type_info);
    void add_namespace_to_scope(TokenIndex token_index, TypeInfo *type_info);
    TypeInfo * get_type_from_scope(TokenIndex token_index);
    TypeInfo * get_type_from_scope_with_string(std::string identifier);
    TypeInfo * get_fn_from_scope(TokenIndex token_index);
    TypeInfo * get_fn_from_scope_with_string(std::string identifier);
    TypeInfo * get_namespace_from_scope(TokenIndex token_index);
    TypeInfo * get_from_scope_with_string(std::string identifier);
};

struct CompilationBundle {
    std::vector<CompilationUnit *> compilation_units;

    CompilationBundle(std::vector<CompilationUnit *> compilation_units);
};

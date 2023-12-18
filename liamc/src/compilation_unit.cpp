//
// Created by jackdelahunt on 11/11/23.
//

#include "compilation_unit.h"
#include "ast.h"
#include "utils.h"

CompilationUnit::CompilationUnit(FileData *file_data, std::vector<Token> token_buffer) {
    this->file_data                   = file_data;
    this->token_buffer                = std::move(token_buffer);
    this->top_level_struct_statements = std::vector<StructStatement *>();
    this->top_level_fn_statements     = std::vector<FnStatement *>();
    this->top_level_import_statements = std::vector<ImportStatement *>();
    this->global_namespace_scope      = Scope();
    this->global_type_scope           = Scope();
    this->global_fn_scope             = Scope();

    this->global_type_scope["void"]   = new VoidTypeInfo();
    this->global_type_scope["bool"]   = new BoolTypeInfo();
    this->global_type_scope["u8"]     = new NumberTypeInfo(8, NumberType::UNSIGNED);
    this->global_type_scope["i8"]     = new NumberTypeInfo(8, NumberType::SIGNED);
    this->global_type_scope["u16"]    = new NumberTypeInfo(16, NumberType::UNSIGNED);
    this->global_type_scope["i16"]    = new NumberTypeInfo(16, NumberType::SIGNED);
    this->global_type_scope["u32"]    = new NumberTypeInfo(32, NumberType::UNSIGNED);
    this->global_type_scope["i32"]    = new NumberTypeInfo(32, NumberType::SIGNED);
    this->global_type_scope["f32"]    = new NumberTypeInfo(32, NumberType::FLOAT);
    this->global_type_scope["u64"]    = new NumberTypeInfo(64, NumberType::UNSIGNED);
    this->global_type_scope["i64"]    = new NumberTypeInfo(64, NumberType::SIGNED);
    this->global_type_scope["f64"]    = new NumberTypeInfo(64, NumberType::FLOAT);
}

Token *CompilationUnit::get_token(TokenIndex token_index) {
    ASSERT(token_index < this->token_buffer.size());
    return &this->token_buffer[token_index];
}

std::string CompilationUnit::get_token_string_from_index(TokenIndex token_index) {
    Token *token_data = &this->token_buffer[token_index];

    std::string token_string;
    token_string.assign(this->file_data->data + token_data->span.start,
                        (token_data->span.end - token_data->span.start) + 1);
    return token_string;
}

ScopeActionStatus CompilationUnit::add_type_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    std::string identifier = this->get_token_string_from_index(token_index);

    if (this->global_type_scope.count(identifier) > 0) {
        return ScopeActionStatus::ALREADY_EXISTS;
    }

    this->global_type_scope[identifier] = type_info;
    return ScopeActionStatus::OK;
}

ScopeActionStatus CompilationUnit::add_fn_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    std::string identifier = this->get_token_string_from_index(token_index);

    if (this->global_fn_scope.count(identifier) > 0) {
        return ScopeActionStatus::ALREADY_EXISTS;
    }

    this->global_fn_scope[identifier] = type_info;
    return ScopeActionStatus::OK;
}

ScopeActionStatus CompilationUnit::add_namespace_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    std::string identifier = this->get_token_string_from_index(token_index);

    if (this->global_namespace_scope.count(identifier) > 0) {
        return ScopeActionStatus::ALREADY_EXISTS;
    }

    this->global_namespace_scope[identifier] = type_info;
    return ScopeActionStatus::OK;
}

TypeInfo *CompilationUnit::get_type_from_scope(TokenIndex token_index) {
    // TODO error checking
    std::string identifier = this->get_token_string_from_index(token_index);
    return this->global_type_scope[identifier];
}

TypeInfo *CompilationUnit::get_type_from_scope_with_string(std::string identifier) {
    return this->global_type_scope[identifier];
}

TypeInfo *CompilationUnit::get_fn_from_scope(TokenIndex token_index) {
    // TODO error checking
    std::string identifier = this->get_token_string_from_index(token_index);
    return this->global_fn_scope[identifier];
}

TypeInfo *CompilationUnit::get_fn_from_scope_with_string(std::string identifier) {
    return this->global_fn_scope[identifier];
}

TypeInfo *CompilationUnit::get_namespace_from_scope(TokenIndex token_index) {
    // TODO error checking
    std::string identifier = this->get_token_string_from_index(token_index);
    return this->global_namespace_scope[identifier];
}

TypeInfo *CompilationUnit::get_from_scope_with_string(std::string identifier) {
    if (this->global_type_scope.count(identifier) > 0) {
        return this->global_type_scope[identifier];
    }

    if (this->global_fn_scope.count(identifier) > 0) {
        return this->global_fn_scope[identifier];
    }

    return NULL;
}

CompilationBundle::CompilationBundle(std::vector<CompilationUnit *> compilation_units) {
    this->compilation_units = compilation_units;
}

Option<u64> CompilationBundle::get_compilation_unit_index_with_path_relative_from(std::string relative_from,
                                                                                  std::string path) {
    std::filesystem::path relative_slash_path = std::filesystem::path(relative_from) / path;
    std::filesystem::path absolute_path       = std::filesystem::weakly_canonical(relative_slash_path);

    for (u64 i = 0; i < this->compilation_units.size(); i++) {
        CompilationUnit *compilation_unit = this->compilation_units[i];
        if (compilation_unit->file_data->absolute_path == absolute_path) {
            return Option(i);
        }
    }

    return Option<u64>();
}

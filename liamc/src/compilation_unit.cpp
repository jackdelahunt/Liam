//
// Created by jackdelahunt on 11/11/23.
//

#include "compilation_unit.h"

CompilationUnit::CompilationUnit(FileData *file_data, std::vector<Token> token_buffer) {
    this->file_data                   = file_data;
    this->token_buffer                = std::move(token_buffer);
    this->top_level_struct_statements = std::vector<StructStatement *>();
    this->top_level_fn_statements     = std::vector<FnStatement *>();
}

Token *CompilationUnit::get_token(TokenIndex token_index) {
    ASSERT(token_index < this->token_buffer.size());
    return &this->token_buffer[token_index];
}

std::string CompilationUnit::get_token_string_from_index(TokenIndex token_index) {
    Token *token_data = &this->token_buffer[token_index];

    std::string token_string;
    token_string.assign(
        this->file_data->data + token_data->span.start, (token_data->span.end - token_data->span.start) + 1
    );
    return token_string;
}

CompilationBundle::CompilationBundle(std::vector<CompilationUnit *> compilation_units) {
    this->compilation_units = compilation_units;
}

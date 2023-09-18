#pragma once

#include <filesystem>
#include <utility>

#include "ast.h"
#include "file.h"

struct CompilationUnit {
    FileData *file_data;
    std::vector<Token> tokens;
    std::vector<TokenData> token_buffer;
    std::vector<ImportStatement *> top_level_import_statements;
    std::vector<StructStatement *> top_level_struct_statements;
    std::vector<FnStatement *> top_level_fn_statements;

    CompilationUnit(FileData *file_data, std::vector<Token> tokens, std::vector<TokenData> token_buffer) {
        this->file_data                   = file_data;
        this->tokens                      = std::move(tokens);
        this->token_buffer                = std::move(token_buffer);
        this->top_level_import_statements = std::vector<ImportStatement *>();
        this->top_level_struct_statements = std::vector<StructStatement *>();
        this->top_level_fn_statements     = std::vector<FnStatement *>();

        ASSERT(this->tokens.size() == this->token_buffer.size());
    }

    std::string get_token_string_from_index(TokenIndex token_index) {
        TokenData *token_data = &this->token_buffer[token_index];

        // TODO: figure out what is best for this
        std::string token_string;
        for (u64 i = token_data->span.start; i <= token_data->span.end; i++)
        {
            token_string.push_back(this->file_data->data[i]);
        }

        return token_string;
    }
};
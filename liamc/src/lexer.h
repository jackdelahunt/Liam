#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compilation_unit.h"
#include "file.h"
#include "liam.h"

bool is_delim(char c);

struct Lexer {
    FileData *file_data;
    u64 current_index;

    std::vector<Token> token_buffer;

    Lexer(FileData *file_data);

    CompilationUnit *lex();
    void next_char();
    char peek();
    std::string get_word();
};

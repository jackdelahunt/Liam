#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "file.h"
#include "liam.h"
#include "compilation_unit.h"

enum class NumberType {
    UNSIGNED,
    SIGNED,
    FLOAT
};

bool is_delim(char c);

struct Lexer {
    FileData *file_data;
    std::vector<Token> tokens;
    i32 current_index;
    i32 current_line;
    i32 current_character;

    Lexer(FileData *file_data);

    CompilationUnit *lex();
    void next_char();
    char peek();
    std::string get_word();
};

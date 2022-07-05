#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "liam.h"

std::vector<char> extract_chars(const char *path);

enum TokenType {
    TOKEN_INT_LITERAL = 0,
    TOKEN_STRING_LITERAL = 1,
    TOKEN_IDENTIFIER = 2,
    TOKEN_LET = 3,
    TOKEN_INSERT = 4,
    TOKEN_FN = 5,
    TOKEN_LOOP = 6,
    TOKEN_PAREN_OPEN = 7,
    TOKEN_PAREN_CLOSE = 8,
    TOKEN_BRACE_OPEN = 9,
    TOKEN_BRACE_CLOSE = 10,
    TOKEN_PLUS = 11,
    TOKEN_STAR = 12,
    TOKEN_ASSIGN = 13,
    TOKEN_SEMI_COLON = 14,
    TOKEN_COMMA = 15,
    TOKEN_COLON = 16,
    TOKEN_RETURN = 17,
    TOKEN_HAT = 18,
    TOKEN_AT = 19,
    TOKEN_STRUCT = 20,
    TOKEN_DOT = 21,
    TOKEN_NEW = 22,
    TOKEN_BREAK = 23,
    TOKEN_IMPORT = 24,
    TOKEN_WALRUS = 25,
    TOKEN_BRACKET_OPEN = 26,
    TOKEN_BRACKET_CLOSE = 27,
    TOKEN_FOR = 28,
    TOKEN_IN = 29,
    TOKEN_FALSE = 30,
    TOKEN_TRUE = 31,
    TOKEN_IF = 32,
    TOKEN_OR = 33,
    TOKEN_AND = 34,
    TOKEN_EQUAL = 35,
    TOKEN_NOT_EQUAL = 36,
    TOKEN_NOT = 37,
    TOKEN_LESS = 38,
    TOKEN_GREATER = 39,
    TOKEN_EXTERN = 40,
};

extern const char *TokenTypeStrings[41];

std::string get_token_type_string(TokenType type);

struct Token {
    TokenType type;
    s32 line;
    s32 character;
    std::string string;

    Token() = default;
    Token(TokenType type, std::string string, s32 line, s32 character);
};

std::ostream &operator<<(std::ostream &os, const Token &token);

bool is_delim(char c);

struct Lexer {
    std::vector<Token> tokens;
    std::vector<char> chars;
    s32 current;
    s32 current_line;
    s32 current_character;
    std::filesystem::path path;

    Lexer(std::filesystem::path path);

    void lex();
    void next_char();
    char peek();
    std::string get_word();
};

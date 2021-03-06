#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "liam.h"

enum NumberType {
    UNSIGNED,
    SIGNED,
    FLOAT
};

enum TokenType {
    TOKEN_NUMBER_LITERAL = 0, // 0
    TOKEN_STRING_LITERAL,     // "hello"
    TOKEN_IDENTIFIER,         // x
    TOKEN_LET,                // let
    TOKEN_INSERT,             // insert
    TOKEN_FN,                 // fn
    TOKEN_PAREN_OPEN,         // (
    TOKEN_PAREN_CLOSE,        // )
    TOKEN_BRACE_OPEN,         // {
    TOKEN_BRACE_CLOSE,        // }
    TOKEN_PLUS,               // +
    TOKEN_MINUS,              // -
    TOKEN_STAR,               // *
    TOKEN_SLASH,              // /
    TOKEN_MOD,                // %
    TOKEN_ASSIGN,             // =
    TOKEN_SEMI_COLON,         // ;
    TOKEN_COMMA,              // ,
    TOKEN_COLON,              // :
    TOKEN_RETURN,             // return
    TOKEN_HAT,                // ^
    TOKEN_AT,                 // @
    TOKEN_STRUCT,             // struct
    TOKEN_DOT,                // .
    TOKEN_NEW,                // new
    TOKEN_BREAK,              // break
    TOKEN_IMPORT,             // import
    TOKEN_BRACKET_OPEN,       // [
    TOKEN_BRACKET_CLOSE,      // ]
    TOKEN_FOR,                // for
    TOKEN_FALSE,              // false
    TOKEN_TRUE,               // true
    TOKEN_IF,                 // if
    TOKEN_ELSE,               // else
    TOKEN_OR,                 // or
    TOKEN_AND,                // and
    TOKEN_EQUAL,              // ==
    TOKEN_NOT_EQUAL,          // !=
    TOKEN_NOT,                // !
    TOKEN_LESS,               // <
    TOKEN_GREATER,            // >
    TOKEN_GREATER_EQUAL,      // >=
    TOKEN_LESS_EQUAL,         // <=
    TOKEN_EXTERN,             // extern
    TOKEN_BAR,                // |
    TOKEN_IS,                 // is
    TOKEN_NULL,               // null
    TOKEN_ENUM,               // enum
};

extern const char *TokenTypeStrings[48];

std::string get_token_type_string(TokenType type);

struct Span {
    u32 line;
    u32 start;
    u32 end;
};

struct Token {
    TokenType type;
    Span span;
    std::string string;

    Token() = default;
    Token(TokenType type, std::string string, u32 line, u32 start);
};

std::ostream &operator<<(std::ostream &os, const Token &token);

bool is_delim(char c);

struct Lexer {
    std::vector<Token> tokens;
    s32 current;
    s32 current_line;
    s32 current_character;
    std::filesystem::path path;

    Lexer(std::filesystem::path path);

    void lex();
    void next_char();
    char peek(std::vector<char> *chars);
    std::string get_word(std::vector<char> *chars);
};

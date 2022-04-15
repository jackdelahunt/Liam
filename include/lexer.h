#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

std::vector<char> extract_chars(const char* path);

enum TokenType {
    TOKEN_INT_LITERAL       = 0,
    TOKEN_STRING_LITERAL    = 1,
    TOKEN_IDENTIFIER        = 2,
    TOKEN_LET               = 3,
    TOKEN_INSERT            = 4,
    TOKEN_FN                = 5,
    TOKEN_PAREN_OPEN        = 6,
    TOKEN_PAREN_CLOSE       = 7,
    TOKEN_BRACE_OPEN        = 8,
    TOKEN_BRACE_CLOSE       = 9,
    TOKEN_PLUS              = 10,
    TOKEN_MULT              = 11,
    TOKEN_EQUAL             = 12,
    TOKEN_SEMI_COLON        = 13,
    TOKEN_COMMA             = 14,
    TOKEN_RETURN            = 15,
};

struct Token
{
    TokenType type;
    std::string string;

    Token() = default;
    Token(TokenType type, std::string string);
};
    
std::ostream& operator<<(std::ostream& os, const Token& token);

bool is_delim(char c);

struct Lexer
{
    std::vector<Token> tokens;
    std::vector<char> chars;
    int current;

    Lexer();

    void lex(const char* path);
    std::string get_word();
};

#pragma once
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include "str.h"
#include "arena.h"

typedef enum TokenType {
    TOKEN_INT_LITERAL = 0,
    TOKEN_STRING_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_LET,
    TOKEN_INSERT,
    TOKEN_FN,
    TOKEN_LOOP,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_ASSIGN,
    TOKEN_SEMI_COLON,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_RETURN,
    TOKEN_HAT,
    TOKEN_AT,
    TOKEN_STRUCT,
    TOKEN_DOT,
    TOKEN_NEW,
    TOKEN_BREAK,
    TOKEN_IMPORT,
    TOKEN_WALRUS,
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_FALSE,
    TOKEN_TRUE,
    TOKEN_IF,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_NOT
} TokenType;

typedef struct Token {
    Slice slice;
    enum TokenType type;
} Token;

typedef struct Lexer {
    Token* tokens;
    size_t count;
} Lexer;

Token make_token(Slice slice, int type);
bool is_delim(char c);
Lexer make_lexer();
Slice get_word(char* start);
void lex(Lexer* lexer, const char* path);
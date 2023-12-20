#pragma once

#include <string>

#include "baseLayer/types.h"
#include "liam.h"

enum class TokenType {
    TOKEN_NUMBER_LITERAL = 0, // 0
    TOKEN_STRING_LITERAL,     // "hello"
    TOKEN_IDENTIFIER,         // x
    TOKEN_LET,                // let
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
    TOKEN_STRUCT,             // struct
    TOKEN_DOT,                // .
    TOKEN_NEW,                // new
    TOKEN_BREAK,              // break
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
    TOKEN_NULL,               // null
    TOKEN_CONTINUE,           // continue
    TOKEN_ZERO,               // zero
    TOKEN_AMPERSAND,          // &
    TOKEN_MATCH,              // match
    TOKEN_IMPORT,             // import
    TOKEN_PRINT,              // print
    TOKEN_ASSERT,             // assert
    TOKEN_WHILE,              // while
};

struct Span {
    // lines start from 0 always
    // start which is the starting character of the span start from 0
    u64 start;
    u64 end;
};

typedef u64 TokenIndex;

struct Token {
    TokenType token_type;
    Span      span;

    Token(TokenType token_type, u64 start, u64 end);
};

std::string get_token_type_string(TokenType type);

#include "token.h"

const char *TokenTypeStrings[49] = {
    "int literal", "str literal", "identifier", "let",   "fn",     "(",    ")",    "{",   "}",      "+",
    "-",           "*",           "/",          "%",     "=",      ";",    ",",    ":",   "return", "^",
    "struct",      ".",           "new",        "break", "import", "[",    "]",    "for", "false",  "true",
    "if",          "else",        "or",         "and",   "==",     "!=",   "!",    "<",   ">",      ">=",
    "<=",          "null",        "continue",   "zero",  "&",      "@tag", "match"};

Token::Token(TokenType token_type, u64 start, u64 end) {
    this->token_type = token_type;
    this->span       = Span{.line = 1, .start = start, .end = end};
}

std::string get_token_type_string(TokenType type) {
    return TokenTypeStrings[(int)type];
}
#include "token.h"

const char *TokenTypeStrings[49] = {"int literal", "str literal",
                                    "identifier",  "let",
                                    "fn",          "(",
                                    ")",           "{",
                                    "}",           "+",
                                    "-",           "*",
                                    "/",           "%",
                                    "=",           ";",
                                    ",",           ":",
                                    "return",      "^",
                                    "struct",      ".",
                                    "new",         "break",
                                    "import",      "[",
                                    "]",           "for",
                                    "false",       "true",
                                    "if",          "else",
                                    "or",          "and",
                                    "==",          "!=",
                                    "!",           "<",
                                    ">",           ">=",
                                    "<=",          "null",
                                    "continue",    "zero",
                                    "&",           "@tag",
                                    "match"};

Token::Token(TokenType type, std::string string, u32 line, u32 start) {

    // we use the index of where the token is in the compilation_unit
    // so lines and characters start at 1
    ASSERT(line > 0);
    //    ASSERT(start > 0);

    this->type   = type;
    this->string = string;
    this->span   = Span{.line = line, .start = start, .end = (u32)(start + string.length())};
}

TokenData::TokenData(TokenType token_type, u64 start, u64 end) {
    this->token_type = token_type;
    this->span       = Span{.line = 1, .start = start, .end = end};
}

std::string get_token_type_string(TokenType type) {
    return TokenTypeStrings[(int)type];
}
#include "token.h"

const char *TokenTypeStrings[50] = {"int Literal", "string Literal",
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
                                    "enum",        "continue",
                                    "zero",        "&",
                                    "@tag",        "match"};

Token::Token(TokenType type, std::string string, u32 line, u32 start) {

    // we use the index of where the token is in the compilation_unit
    // so lines and characters start at 1
    ASSERT(line > 0);
    ASSERT(start > 0);

    this->type   = type;
    this->string = string;
    this->span   = Span{.line = line, .start = start, .end = (u32)(start + string.length())};
}

std::string get_token_type_string(TokenType type) {
    return TokenTypeStrings[(int)type];
}
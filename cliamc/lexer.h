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

Token make_token(Slice slice, int type) {
    return (Token){slice, type};
}

typedef struct Lexer {
    Token* tokens;
    size_t count;
} Lexer;

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == EOF || c == '\0' || c == ';' || c == '"' || c == '(' || c == ')';
}

Lexer make_lexer() {
    return (Lexer){NULL, 0};
}

Slice get_word(char* start) {
    size_t index = 0;
    char c = start[index];
    while(!is_delim(c)) {
        index++;
        c = start[index];
    }

    return make_slice(start, index);
}

void lex(Lexer* lexer, const char* path) {
    Str source = read_file(path);
    lexer->tokens = ALLOC(sizeof(Token) * source.length);

    size_t index = 0;
    while(index < source.length) {
        char current_char = char_at_string(&source, index);
        bool is_single = true;

        switch (current_char) {
            case ' ':
            case '\n':
            case '\t':
                index++;
                continue;
                break;
            case '+':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PLUS);
                break;
            case '(':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PAREN_OPEN);
                break;
            case ')':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PAREN_CLOSE);
                break;
            case '{':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACE_OPEN);
                break;
            case '}':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACE_CLOSE);
                break;
            case '*':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_STAR);
                break;
            case '=':
                if(char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_EQUAL);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_ASSIGN);
                break;
            case ';':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_SEMI_COLON);
                break;
            case ',':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_COMMA);
                break;
            case ':':
                if(char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_WALRUS);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_COLON);
                break;
            case '^':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_HAT);
                break;
            case '@':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_AT);
                break;
            case '.':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_DOT);
                break;
            case '[':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACKET_OPEN);
                break;
            case ']':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACKET_CLOSE);
                break;
            case '!':
                if(char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_NOT_EQUAL);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_NOT);
                break;
            case '"': {
                size_t start = index;
                index++;
                while(char_at_string(&source, index) != '"') {
                    if(index >= source.length) {
                        assert("Unexpected EOF string does not end... like my pain");
                    }
                    index++;
                }

                size_t length = (index - 1) - start;
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[start + 1], length), TOKEN_STRING_LITERAL);
                index++; // passover quote at the end
            }
                break;
            case '#': {
                while (current_char != '\n' && index < source.length) {
                    current_char = char_at_string(&source, index);
                    index++;
                }

                continue;
            }
                break;
            default:
                is_single = false;
        }

        if(is_single) {
            lexer->count++;
            index++;
            continue;
        }

        Slice word = get_word(&source.buffer[index]);
        index += word.length;

        if(compare_slice(&word, "return")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_RETURN);
        } else if(compare_slice(&word, "let")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_LET);
        } else if(compare_slice(&word, "insert")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_INSERT);
        } else if(compare_slice(&word, "fn")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FN);
        } else if(compare_slice(&word, "loop")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_LOOP);
        }  else if(compare_slice(&word, "struct")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_STRUCT);
        } else if(compare_slice(&word, "new")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_NEW);
        } else if(compare_slice(&word, "break")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_BREAK);
        } else if(compare_slice(&word, "import")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IMPORT);
        } else if(compare_slice(&word, "for")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FOR);
        } else if(compare_slice(&word, "in")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IN);
        } else if(compare_slice(&word, "false")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FALSE);
        } else if(compare_slice(&word, "true")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_TRUE);
        } else if(compare_slice(&word, "if")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IF);
        } else if(compare_slice(&word, "or")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_OR);
        } else if(compare_slice(&word, "and")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_AND);
        }

        int n = 0;
        if(slice_to_int(&word, &n)) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_INT_LITERAL);
        } else {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IDENTIFIER);
        }

        lexer->count++;
    }
}

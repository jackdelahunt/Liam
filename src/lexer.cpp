#pragma once
#include "lexer.h"

std::vector<char> extract_chars(const char* path) {
    auto vec = std::vector<char>();

    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        std::cout << "cannot open file";
    }

    for (int i = file.get(); i != EOF; i = file.get())
    {
        vec.push_back((char)i);
    }

    file.close();
    return vec;
}

Token::Token(TokenType type, std::string string) {
    this->type = type;
    this->string = string;
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << token.string;
    return os;
}

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',';
}

Lexer::Lexer() {
    tokens = std::vector<Token>();
    current = 0;
 }

void Lexer::lex(const char* path) {
    chars = extract_chars(path);
    for (; current < chars.size(); current++) {
        char c = chars.at(current);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
            break;
        case '+':
            tokens.push_back(Token(TokenType::TOKEN_PLUS, "+"));
            break;
        case '*':
            tokens.push_back(Token(TokenType::TOKEN_MULT, "*"));
            break;
        case '=':
            tokens.push_back(Token(TokenType::TOKEN_EQUAL, "="));
            break;
        case ';':
            tokens.push_back(Token(TokenType::TOKEN_SEMI_COLON, ";"));
            break;
        case '(':
            tokens.push_back(Token(TokenType::TOKEN_PAREN_OPEN, "("));
            break;
        case ')':
            tokens.push_back(Token(TokenType::TOKEN_PAREN_CLOSE, ")"));
            break;
        case '{':
            tokens.push_back(Token(TokenType::TOKEN_BRACE_OPEN, "{"));
            break;
        case '}':
            tokens.push_back(Token(TokenType::TOKEN_BRACE_CLOSE, "}"));
            break;
        case ',':
            tokens.push_back(Token(TokenType::TOKEN_COMMA, ","));
            break;
        case '#':
            while (current < chars.size() && chars.at(current) != '\n') {
                current++;
            }
            break;
        case '"':
        {
            current++; // go to next character after quote
            std::string str = std::string();
            while (current < chars.size() && chars.at(current) != '"') {
                str.append(std::string(1, chars.at(current)));
                current++;
            }
            tokens.push_back(Token(TokenType::TOKEN_STRING_LITERAL, str));
        }
        break;
        default:
            auto word = get_word();

            // check keywords
            if (word == "let") {
                tokens.push_back(Token(TokenType::TOKEN_LET, word));
                continue;
            }

            // check keywords
            if (word == "insert") {
                tokens.push_back(Token(TokenType::TOKEN_INSERT, word));
                continue;
            }

            // check keywords
            if (word == "fn") {
                tokens.push_back(Token(TokenType::TOKEN_FN, word));
                continue;
            }

            // check keywords
            if (word == "return") {
                tokens.push_back(Token(TokenType::TOKEN_RETURN, word));
                continue;
            }

            // check numbers
            try
            {
                int i = std::stoi(word);
                tokens.push_back(Token(TokenType::TOKEN_INT_LITERAL, word));
                continue;
            }
            catch (const std::exception& e) {}

            tokens.push_back(Token(TokenType::TOKEN_IDENTIFIER, word));

            break;
        }
    }
}

std::string Lexer::get_word() {
    std::string word = std::string();
    while (current < chars.size() && !is_delim(chars.at(current))) {
        word.append(1, chars.at(current));
        current++;
    }
    current--; // sets current as it will be iterated once after this
    return word;
}
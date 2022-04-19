#pragma once
#include "lexer.h"

const char* TokenTypeStrings[21] = {
    "int Literal",
    "string Literal",
    "identifier",
    "let",
    "insert",
    "fn",
    "loop",
    "(",
    ")",
    "{",
    "}",
    "+",
    "*",
    "=",
    ";",
    ",",
    ":",
    "return",
    "type",
    "^",
    "@"
};

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

Token::Token(TokenType type, std::string string, int line, int character) {
    this->type = type;
    this->string = string;
    this->line = line;
    this->character = character;
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << token.string;
    return os;
}

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' || c == '=' || c == '+' || c == '^' || c == '@' || c == '*';
}

Lexer::Lexer() {
    tokens = std::vector<Token>();
    current = 0;
    current_line = 1;
    current_character = 0;
 }

void Lexer::lex(const char* path) {
    chars = extract_chars(path);
    for (; current < chars.size(); next_char()) {
        char c = chars.at(current);
        switch (c)
        {
        case '\n':
            current_line++;
            current_character = 0;
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '+':
            tokens.push_back(Token(TokenType::TOKEN_PLUS, "+", current_line, current_character));
            break;
        case '*':
            tokens.push_back(Token(TokenType::TOKEN_STAR, "*", current_line, current_character));
            break;
        case '=':
            tokens.push_back(Token(TokenType::TOKEN_EQUAL, "=", current_line, current_character));
            break;
        case ';':
            tokens.push_back(Token(TokenType::TOKEN_SEMI_COLON, ";", current_line, current_character));
            break;
        case '(':
            tokens.push_back(Token(TokenType::TOKEN_PAREN_OPEN, "(", current_line, current_character));
            break;
        case ')':
            tokens.push_back(Token(TokenType::TOKEN_PAREN_CLOSE, ")", current_line, current_character));
            break;
        case '{':
            tokens.push_back(Token(TokenType::TOKEN_BRACE_OPEN, "{", current_line, current_character));
            break;
        case '}':
            tokens.push_back(Token(TokenType::TOKEN_BRACE_CLOSE, "}", current_line, current_character));
            break;
        case ',':
            tokens.push_back(Token(TokenType::TOKEN_COMMA, ",", current_line, current_character));
            break;
        case ':':
            tokens.push_back(Token(TokenType::TOKEN_COLON, ":", current_line, current_character));
            break;
        case '^':
            tokens.push_back(Token(TokenType::TOKEN_HAT, "^", current_line, current_character));
            break;
        case '@':
            tokens.push_back(Token(TokenType::TOKEN_AT, "@", current_line, current_character));
            break;
        case '#':
            while (current < chars.size() && chars.at(current) != '\n') {
                next_char();
            }
            break;
        case '"':
        {
            next_char();
            std::string str = std::string();
            while (current < chars.size() && chars.at(current) != '"') {
                // skip back slash and accept next char
                if (chars.at(current) == '\\') {
                    next_char();
                    str.append(std::string(1, chars.at(current)));
                    next_char();
                    continue;
                }
                str.append(std::string(1, chars.at(current)));
                next_char();
            }
            tokens.push_back(Token(TokenType::TOKEN_STRING_LITERAL, str, current_line, current_character));
        }
        break;
        default:
            auto word = get_word();

            // check keywords
            if (word == "let") {
                tokens.push_back(Token(TokenType::TOKEN_LET, word, current_line, current_character));
                continue;
            }

            if (word == "insert") {
                tokens.push_back(Token(TokenType::TOKEN_INSERT, word, current_line, current_character));
                continue;
            }

            if (word == "fn") {
                tokens.push_back(Token(TokenType::TOKEN_FN, word, current_line, current_character));
                continue;
            }

            if (word == "return") {
                tokens.push_back(Token(TokenType::TOKEN_RETURN, word, current_line, current_character));
                continue;
            }

            if (word == "u64" || word == "string") {
                tokens.push_back(Token(TokenType::TOKEN_TYPE, word, current_line, current_character));
                continue;
            }

            if (word == "void") {
                tokens.push_back(Token(TokenType::TOKEN_TYPE, word, current_line, current_character));
                continue;
            }

            if (word == "loop") {
                tokens.push_back(Token(TokenType::TOKEN_LOOP, word, current_line, current_character));
                continue;
            }

            // check numbers
            try
            {
                int i = std::stoi(word);
                tokens.push_back(Token(TokenType::TOKEN_INT_LITERAL, word, current_line, current_character));
                continue;
            }
            catch (const std::exception& e) {}

            tokens.push_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, current_character));

            break;
        }
    }
}

void Lexer::next_char() {
    current++;
    current_character++;
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
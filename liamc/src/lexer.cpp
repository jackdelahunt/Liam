#include "lexer.h"
#include <filesystem>
#include "liam.h"

const char* TokenTypeStrings[38] = {
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
    "return_type",
    "^",
    "@",
    "struct",
    ".",
    "new",
    "break",
    ":=",
    "[",
    "]",
    "for",
    "in",
    "false",
    "true",
    "if",
    "or",
    "and",
    "==",
    "!=",
    "!",
};

std::vector<char> extract_chars(const char* path) {
    auto vec = std::vector<char>();

    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        std::cout << "cannot open file";
    }

    for (s32 i = file.get(); i != EOF; i = file.get())
    {
        vec.push_back((char)i);
    }

    file.close();
    return vec;
}

Token::Token(TokenType type, std::string string, s32 line, s32 character) {
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
    return c == ' ' || c == '\n' || c == ';' || c == '(' || 
        c == ')' || c == '{' || c == '}' || c == ',' || 
        c == ':' || c == '=' || c == '+' || c == '^' || 
        c == '@' || c == '*' || c == '.' || c == '[' ||
        c == ']' || c == '!';
}

Lexer::Lexer(std::filesystem::path path) {
    tokens = std::vector<Token>();
    current = 0;
    current_line = 1;
    current_character = 0;
    this->path = path;
 }

void Lexer::lex() {
    chars = extract_chars(path.string().c_str());
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
            tokens.emplace_back(TokenType::TOKEN_PLUS, "+", current_line, current_character);
            break;
        case '*':
            tokens.emplace_back(TokenType::TOKEN_STAR, "*", current_line, current_character);
            break;
        case '=':
            if(peek() == '=') {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_EQUAL, "==", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_ASSIGN, "=", current_line, current_character));
            break;
        case ';':
            tokens.emplace_back(Token(TokenType::TOKEN_SEMI_COLON, ";", current_line, current_character));
            break;
        case '(':
            tokens.emplace_back(Token(TokenType::TOKEN_PAREN_OPEN, "(", current_line, current_character));
            break;
        case ')':
            tokens.emplace_back(Token(TokenType::TOKEN_PAREN_CLOSE, ")", current_line, current_character));
            break;
        case '{':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACE_OPEN, "{", current_line, current_character));
            break;
        case '}':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACE_CLOSE, "}", current_line, current_character));
            break;
        case ',':
            tokens.emplace_back(Token(TokenType::TOKEN_COMMA, ",", current_line, current_character));
            break;
        case '[':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACKET_OPEN, "[", current_line, current_character));
            break;
        case ']':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACKET_CLOSE, "]", current_line, current_character));
            break;
        case ':':
            if(peek() == '=') {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_WALRUS, ":=", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_COLON, ":", current_line, current_character));
            break;
        case '^':
            tokens.emplace_back(Token(TokenType::TOKEN_HAT, "^", current_line, current_character));
            break;
        case '@':
            tokens.emplace_back(Token(TokenType::TOKEN_AT, "@", current_line, current_character));
            break;
        case '.':
            tokens.emplace_back(Token(TokenType::TOKEN_DOT, ".", current_line, current_character));
            break;
        case '!':
            if(peek() == '=') {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_NOT_EQUAL, "!=", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_NOT, "!", current_line, current_character));
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
            s32 word_start = current_character;
            auto word = get_word();

            // check keywords
            if (word == "let") {
                tokens.emplace_back(Token(TokenType::TOKEN_LET, word, current_line, word_start));
                continue;
            }

            if (word == "insert") {
                tokens.emplace_back(Token(TokenType::TOKEN_INSERT, word, current_line, word_start));
                continue;
            }

            if (word == "fn") {
                tokens.emplace_back(Token(TokenType::TOKEN_FN, word, current_line, word_start));
                continue;
            }

            if (word == "return") {
                tokens.emplace_back(Token(TokenType::TOKEN_RETURN, word, current_line, word_start));
                continue;
            }

            if (word == "loop") {
                tokens.emplace_back(Token(TokenType::TOKEN_LOOP, word, current_line, word_start));
                continue;
            }

            if (word == "struct") {
                tokens.emplace_back(Token(TokenType::TOKEN_STRUCT, word, current_line, word_start));
                continue;
            }

            if (word == "new") {
                tokens.emplace_back(Token(TokenType::TOKEN_NEW, word, current_line, word_start));
                continue;
            }

            if (word == "break") {
                tokens.emplace_back(Token(TokenType::TOKEN_BREAK, word, current_line, word_start));
                continue;
            }

            if (word == "import") {
                tokens.emplace_back(Token(TokenType::TOKEN_IMPORT, word, current_line, word_start));
                continue;
            }

            if (word == "for") {
                tokens.emplace_back(Token(TokenType::TOKEN_FOR, word, current_line, word_start));
                continue;
            }

            if (word == "in") {
                tokens.emplace_back(Token(TokenType::TOKEN_IN, word, current_line, word_start));
                continue;
            }

            if (word == "if") {
                tokens.emplace_back(Token(TokenType::TOKEN_IF, word, current_line, word_start));
                continue;
            }

            if (word == "and") {
                tokens.emplace_back(Token(TokenType::TOKEN_AND, word, current_line, word_start));
                continue;
            }

            if (word == "or") {
                tokens.emplace_back(Token(TokenType::TOKEN_OR, word, current_line, word_start));
                continue;
            }

            // built in types
            if (word == "void") {
                tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));
                continue;
            }

            if (word == "u64") {
                tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));
                continue;
            }

            if (word == "char") {
                tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));
                continue;
            }

            if (word == "true") {
                tokens.emplace_back(Token(TokenType::TOKEN_TRUE, word, current_line, word_start));
                continue;
            }

            if (word == "false") {
                tokens.emplace_back(Token(TokenType::TOKEN_FALSE, word, current_line, word_start));
                continue;
            }

            // check numbers
            try
            {
                s32 i = std::stoi(word);
                tokens.emplace_back(Token(TokenType::TOKEN_INT_LITERAL, word, current_line, word_start));
                continue;
            }
            catch (const std::exception& e) {}

            tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));

            break;
        }
    }
}

void Lexer::next_char() {
    current++;
    current_character++;
}

char Lexer::peek() {
    return chars.at(current + 1);
}

std::string Lexer::get_word() {
    std::string word = std::string();
    while (current < chars.size() && !is_delim(chars.at(current))) {
        word.append(1, chars.at(current));
        next_char();
    }
    current--; // sets current as it will be iterated once after this
    return word;
}
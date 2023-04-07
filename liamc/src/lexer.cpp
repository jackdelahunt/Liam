#include "lexer.h"

#include <filesystem>

#include "file.h"
#include "liam.h"
#include "utils.h"

const char *TokenTypeStrings[49] = {"int Literal", "string Literal",
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
                                    "@tag"};

Token::Token(TokenType type, std::string string, u32 line, u32 start) {

    // we use the index of where the token is in the file
    // so lines and characters start at 1
    ASSERT(line > 0);
    ASSERT(start > 0);

    this->type   = type;
    this->string = string;
    this->span   = Span{.line = line, .start = start, .end = (u32)(start + string.length())};
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
    os << token.string;
    return os;
}

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' ||
           c == '=' || c == '+' || c == '^' || c == '&' || c == '*' || c == '.' || c == '[' || c == ']' || c == '!' ||
           c == '<' || c == '>' || c == '|' || c == '-' || c == '/' || c == '%';
}

Lexer::Lexer(std::filesystem::path path) {
    tokens            = std::vector<Token>();
    current_index     = 0;
    current_line      = 1;
    current_character = 1;
    this->path        = path;
}

void Lexer::lex() {
    auto as_string  = path.string();
    auto file_data  = FileManager::load(&as_string);
    char *chars     = file_data->data;
    u64 data_length = file_data->data_length;

    // pre allocate some of the token vector
    u64 token_vec_start_size = data_length * 0.45;
    if (token_vec_start_size > this->tokens.capacity())
    { this->tokens.reserve(token_vec_start_size); }

    ASSERT(chars);

    for (; this->current_index < data_length; next_char())
    {

        char c = chars[this->current_index];
        switch (c)
        {
        case '\n':
            this->current_line++;

            // while all character locations start at 1, we need to set this to 0
            // because when we break from the switch and go to the next iteration
            // it will iterate the current_character by 1 setting it back to the
            // desired starting point
            this->current_character = 0;
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '+':
            tokens.emplace_back(TokenType::TOKEN_PLUS, "+", current_line, current_character);
            break;
        case '-':
            tokens.emplace_back(TokenType::TOKEN_MINUS, "-", current_line, current_character);
            break;
        case '*':
            tokens.emplace_back(TokenType::TOKEN_STAR, "*", current_line, current_character);
            break;
        case '/':
            if (peek(file_data) == '/')
            {
                while (this->current_index < data_length && chars[this->current_index] != '\n')
                { next_char(); }
                current_line++;
                break;
            }
            tokens.emplace_back(TokenType::TOKEN_SLASH, "/", current_line, current_character);
            break;
        case '%':
            tokens.emplace_back(TokenType::TOKEN_MOD, "%", current_line, current_character);
            break;
        case '=':
            if (peek(file_data) == '=')
            {
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
            tokens.emplace_back(Token(TokenType::TOKEN_COLON, ":", current_line, current_character));
            break;
        case '^':
            tokens.emplace_back(Token(TokenType::TOKEN_HAT, "^", current_line, current_character));
            break;
        case '&':
            tokens.emplace_back(Token(TokenType::TOKEN_AMPERSAND, "&", current_line, current_character));
            break;
        case '.':
            tokens.emplace_back(Token(TokenType::TOKEN_DOT, ".", current_line, current_character));
            break;
        case '<':
            if (peek(file_data) == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_LESS_EQUAL, "<=", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_LESS, "<", current_line, current_character));
            break;
        case '>':
            if (peek(file_data) == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_GREATER_EQUAL, ">=", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_GREATER, ">", current_line, current_character));
            break;
        case '!':
            if (peek(file_data) == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_NOT_EQUAL, "!=", current_line, current_character);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_NOT, "!", current_line, current_character));
            break;
        case '"': {
            u64 start       = current_character;
            std::string str = std::string();

            next_char();
            while (current_index < data_length && chars[current_index] != '"')
            {
                // skip back slash and accept next char
                if (chars[current_index] == '\\')
                {
                    str.append(std::string(1, chars[current_index]));
                    next_char();
                }

                str.append(std::string(1, chars[current_index]));
                next_char();
            }
            tokens.emplace_back(TokenType::TOKEN_STRING_LITERAL, str, current_line, start);
        }
        break;
        default:
            i32 word_start = current_character;
            auto word      = get_word(file_data);

            if (word.data()[0] == '@')
            {
                tokens.emplace_back(Token(TokenType::TOKEN_TAG, word, current_line, word_start));
                continue;
            }

            // check keywords
            if (compare_string(word, "let"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_LET, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "fn"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FN, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "return"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_RETURN, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "struct"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_STRUCT, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "new"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_NEW, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "continue"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_CONTINUE, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "import"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_IMPORT, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "for"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FOR, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "if"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_IF, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "else"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ELSE, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "and"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_AND, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "or"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_OR, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "enum"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ENUM, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "true"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_TRUE, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "false"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FALSE, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "null"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_NULL, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "zero"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ZERO, word, current_line, word_start));
                continue;
            }

            if (compare_string(word, "break"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_BREAK, word, current_line, word_start));
                continue;
            }

            // check numbers
            if (is_digit(word.at(0)) || word.at(0) == '-')
            {

                // TODO:
                // if this word is a number then it might have got stuck on a .
                // this means the word is actually shorter than the actual number
                // to stop this we revert the current_index location and read until we
                // find a delimiter but not including . or -

                tokens.emplace_back(Token(TokenType::TOKEN_NUMBER_LITERAL, word, current_line, word_start));
                continue;
            }

            // must be an identifier
            tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));

            break;
        }
    }
}

void Lexer::next_char() {
    this->current_index++;
    this->current_character++;
}

char Lexer::peek(FileData *file_data) {
    return file_data->data[this->current_index + 1];
}

std::string Lexer::get_word(FileData *file_data) {
    std::string word = std::string();
    while (this->current_index < file_data->data_length && !is_delim(file_data->data[this->current_index]))
    {
        word.append(1, file_data->data[this->current_index]);
        next_char();
    }

    this->current_index--; // it will be iterated once after this
    current_character--;   // it will be iterated once after this

    return word;
}

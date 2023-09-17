#include "lexer.h"

#include <filesystem>

#include "file.h"
#include "liam.h"
#include "utils.h"

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' ||
           c == '=' || c == '+' || c == '^' || c == '&' || c == '*' || c == '.' || c == '[' || c == ']' || c == '!' ||
           c == '<' || c == '>' || c == '|' || c == '-' || c == '/' || c == '%';
}

Lexer::Lexer(FileData *file_data) {
    this->file_data         = file_data;
    this->tokens            = std::vector<Token>();
    this->current_index     = 0;
    this->current_line      = 1;
    this->current_character = 1;
    this->token_buffer      = std::vector<TokenData>();

    ASSERT(this->file_data->data);
}

CompilationUnit *Lexer::lex() {
    // pre allocate some of the token vector
    u64 token_vec_start_size = this->file_data->data_length * 0.45;
    if (token_vec_start_size > this->tokens.capacity())
    {
        this->tokens.reserve(token_vec_start_size);
    }

    for (; this->current_index < this->file_data->data_length; next_char())
    {
        char c = this->file_data->data[this->current_index];
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
            this->token_buffer.emplace_back(TokenType::TOKEN_PLUS, this->current_index, this->current_index);
            break;
        case '-':
            tokens.emplace_back(TokenType::TOKEN_MINUS, "-", current_line, current_character);
            this->token_buffer.emplace_back(TokenType::TOKEN_MINUS, this->current_index, this->current_index);
            break;
        case '*':
            tokens.emplace_back(TokenType::TOKEN_STAR, "*", current_line, current_character);
            this->token_buffer.emplace_back(TokenType::TOKEN_STAR, this->current_index, this->current_index);
            break;
        case '/':
            if (peek() == '/')
            {
                while (this->current_index < this->file_data->data_length &&
                       this->file_data->data[this->current_index] != '\n')
                {
                    next_char();
                }
                current_line++;
                break;
            }
            tokens.emplace_back(TokenType::TOKEN_SLASH, "/", current_line, current_character);
            this->token_buffer.emplace_back(TokenType::TOKEN_SLASH, this->current_index, this->current_index);
            break;
        case '%':
            tokens.emplace_back(TokenType::TOKEN_MOD, "%", current_line, current_character);
            this->token_buffer.emplace_back(TokenType::TOKEN_MOD, this->current_index, this->current_index);
            break;
        case '=':
            if (peek() == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_EQUAL, "==", current_line, current_character);
                this->token_buffer.emplace_back(TokenType::TOKEN_EQUAL, this->current_index - 1, this->current_index);
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_ASSIGN, "=", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_ASSIGN, this->current_index, this->current_index);
            break;
        case ';':
            tokens.emplace_back(Token(TokenType::TOKEN_SEMI_COLON, ";", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_SEMI_COLON, this->current_index, this->current_index);
            break;
        case '(':
            tokens.emplace_back(Token(TokenType::TOKEN_PAREN_OPEN, "(", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_PAREN_OPEN, this->current_index, this->current_index);
            break;
        case ')':
            tokens.emplace_back(Token(TokenType::TOKEN_PAREN_CLOSE, ")", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_PAREN_CLOSE, this->current_index, this->current_index);
            break;
        case '{':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACE_OPEN, "{", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACE_OPEN, this->current_index, this->current_index);
            break;
        case '}':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACE_CLOSE, "}", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACE_CLOSE, this->current_index, this->current_index);
            break;
        case ',':
            tokens.emplace_back(Token(TokenType::TOKEN_COMMA, ",", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_COMMA, this->current_index, this->current_index);
            break;
        case '[':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACKET_OPEN, "[", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACKET_OPEN, this->current_index, this->current_index);
            break;
        case ']':
            tokens.emplace_back(Token(TokenType::TOKEN_BRACKET_CLOSE, "]", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACKET_CLOSE, this->current_index, this->current_index);
            break;
        case ':':
            tokens.emplace_back(Token(TokenType::TOKEN_COLON, ":", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_COLON, this->current_index, this->current_index);
            break;
        case '^':
            tokens.emplace_back(Token(TokenType::TOKEN_HAT, "^", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_HAT, this->current_index, this->current_index);
            break;
        case '&':
            tokens.emplace_back(Token(TokenType::TOKEN_AMPERSAND, "&", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_AMPERSAND, this->current_index, this->current_index);
            break;
        case '.':
            tokens.emplace_back(Token(TokenType::TOKEN_DOT, ".", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_DOT, this->current_index, this->current_index);
            break;
        case '<':
            if (peek() == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_LESS_EQUAL, "<=", current_line, current_character);
                this->token_buffer.emplace_back(
                    TokenType::TOKEN_LESS_EQUAL, this->current_index - 1, this->current_index
                );
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_LESS, "<", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_LESS, this->current_index, this->current_index);
            break;
        case '>':
            if (peek() == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_GREATER_EQUAL, ">=", current_line, current_character);
                this->token_buffer.emplace_back(
                    TokenType::TOKEN_GREATER_EQUAL, this->current_index - 1, this->current_index
                );
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_GREATER, ">", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_GREATER, this->current_index, this->current_index);
            break;
        case '!':
            if (peek() == '=')
            {
                next_char();
                tokens.emplace_back(TokenType::TOKEN_NOT_EQUAL, "!=", current_line, current_character);
                this->token_buffer.emplace_back(
                    TokenType::TOKEN_NOT_EQUAL, this->current_index - 1, this->current_index
                );
                break;
            }
            tokens.emplace_back(Token(TokenType::TOKEN_NOT, "!", current_line, current_character));
            this->token_buffer.emplace_back(TokenType::TOKEN_NOT, this->current_index, this->current_index);
            break;
        case '"': {
            u64 start       = current_character;
            std::string str = std::string();

            next_char();
            while (current_index < this->file_data->data_length && this->file_data->data[current_index] != '"')
            {
                // skip back slash and accept next char
                if (this->file_data->data[current_index] == '\\')
                {
                    str.append(std::string(1, this->file_data->data[current_index]));
                    next_char();
                }

                str.append(std::string(1, this->file_data->data[current_index]));
                next_char();
            }
            tokens.emplace_back(TokenType::TOKEN_STRING_LITERAL, str, current_line, start);
            this->token_buffer.emplace_back(TokenType::TOKEN_STRING_LITERAL, start, this->current_index);
        }
        break;
        default:
            i32 word_start = this->current_index;
            auto word      = get_word();

            ASSERT(word.length() > 0);

            if (word.data()[0] == '@')
            {
                tokens.emplace_back(Token(TokenType::TOKEN_TAG, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_TAG, word_start, (word_start - 1) + word.length());
                continue;
            }

            // check keywords
            if (compare_string(word, "let"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_LET, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_LET, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "fn"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FN, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_FN, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "return"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_RETURN, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_RETURN, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "struct"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_STRUCT, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_STRUCT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "new"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_NEW, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_NEW, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "continue"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_CONTINUE, word, current_line, word_start));
                this->token_buffer.emplace_back(
                    TokenType::TOKEN_CONTINUE, word_start, (word_start - 1) + word.length()
                );
                continue;
            }

            if (compare_string(word, "import"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_IMPORT, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_IMPORT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "for"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FOR, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_FOR, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "if"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_IF, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_IF, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "else"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ELSE, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_ELSE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "and"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_AND, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_AND, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "or"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_OR, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_OR, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "enum"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ENUM, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_ENUM, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "true"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_TRUE, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_TRUE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "false"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_FALSE, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_FALSE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "null"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_NULL, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_NULL, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "zero"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_ZERO, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_ZERO, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "break"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_BREAK, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_BREAK, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "match"))
            {
                tokens.emplace_back(Token(TokenType::TOKEN_MATCH, word, current_line, word_start));
                this->token_buffer.emplace_back(TokenType::TOKEN_MATCH, word_start, (word_start - 1) + word.length());
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
                this->token_buffer.emplace_back(
                    TokenType::TOKEN_NUMBER_LITERAL, word_start, (word_start - 1) + word.length()
                );
                continue;
            }

            // must be an identifier
            tokens.emplace_back(Token(TokenType::TOKEN_IDENTIFIER, word, current_line, word_start));
            this->token_buffer.emplace_back(TokenType::TOKEN_IDENTIFIER, word_start, (word_start - 1) + word.length());

            break;
        }
    }

    return new CompilationUnit(this->file_data, this->tokens, this->token_buffer);
}

void Lexer::next_char() {
    this->current_index++;
    this->current_character++;
}

char Lexer::peek() {
    return this->file_data->data[this->current_index + 1];
}

std::string Lexer::get_word() {
    std::string word = std::string();
    while (this->current_index < this->file_data->data_length && !is_delim(this->file_data->data[this->current_index]))
    {
        word.append(1, this->file_data->data[this->current_index]);
        next_char();
    }

    this->current_index--; // it will be iterated once after this
    current_character--;   // it will be iterated once after this

    return word;
}

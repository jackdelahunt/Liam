#include "lexer.h"

#include "file.h"
#include "utils.h"

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' ||
           c == '=' || c == '+' || c == '^' || c == '&' || c == '*' || c == '.' || c == '[' || c == ']' || c == '!' ||
           c == '<' || c == '>' || c == '|' || c == '-' || c == '/' || c == '%' || c == '\r' || c == '\t' || c == '\0';
}

Lexer::Lexer(FileData *file_data) {
    this->file_data     = file_data;
    this->current_index = 0;
    this->token_buffer  = std::vector<Token>();

    ASSERT(this->file_data->data);
}

CompilationUnit *Lexer::lex() {
    // pre allocate some of the token vector
    u64 token_vec_start_size = this->file_data->data_length * 0.45;
    if (token_vec_start_size > this->token_buffer.capacity()) {
        this->token_buffer.reserve(token_vec_start_size);
    }

    for (; this->current_index < this->file_data->data_length; next_char()) {
        char c = this->file_data->data[this->current_index];
        switch (c) {
        case '\n':
        case ' ':
        case '\r':
        case '\t':
            break;
        case '+':
            this->token_buffer.emplace_back(TokenType::TOKEN_PLUS, this->current_index, this->current_index);
            break;
        case '-':
            this->token_buffer.emplace_back(TokenType::TOKEN_MINUS, this->current_index, this->current_index);
            break;
        case '*':
            this->token_buffer.emplace_back(TokenType::TOKEN_STAR, this->current_index, this->current_index);
            break;
        case '/':
            if (peek() == '/') {
                while (this->current_index < this->file_data->data_length &&
                       this->file_data->data[this->current_index] != '\n') {
                    next_char();
                }
                break;
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_SLASH, this->current_index, this->current_index);
            break;
        case '%':
            this->token_buffer.emplace_back(TokenType::TOKEN_MOD, this->current_index, this->current_index);
            break;
        case '=':
            if (peek() == '=') {
                next_char();
                this->token_buffer.emplace_back(TokenType::TOKEN_EQUAL, this->current_index - 1, this->current_index);
                break;
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_ASSIGN, this->current_index, this->current_index);
            break;
        case ';':
            this->token_buffer.emplace_back(TokenType::TOKEN_SEMI_COLON, this->current_index, this->current_index);
            break;
        case '(':
            this->token_buffer.emplace_back(TokenType::TOKEN_PAREN_OPEN, this->current_index, this->current_index);
            break;
        case ')':
            this->token_buffer.emplace_back(TokenType::TOKEN_PAREN_CLOSE, this->current_index, this->current_index);
            break;
        case '{':
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACE_OPEN, this->current_index, this->current_index);
            break;
        case '}':
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACE_CLOSE, this->current_index, this->current_index);
            break;
        case ',':
            this->token_buffer.emplace_back(TokenType::TOKEN_COMMA, this->current_index, this->current_index);
            break;
        case '[':
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACKET_OPEN, this->current_index, this->current_index);
            break;
        case ']':
            this->token_buffer.emplace_back(TokenType::TOKEN_BRACKET_CLOSE, this->current_index, this->current_index);
            break;
        case ':':
            this->token_buffer.emplace_back(TokenType::TOKEN_COLON, this->current_index, this->current_index);
            break;
        case '^':
            this->token_buffer.emplace_back(TokenType::TOKEN_HAT, this->current_index, this->current_index);
            break;
        case '&':
            this->token_buffer.emplace_back(TokenType::TOKEN_AMPERSAND, this->current_index, this->current_index);
            break;
        case '.':
            this->token_buffer.emplace_back(TokenType::TOKEN_DOT, this->current_index, this->current_index);
            break;
        case '<':
            if (peek() == '=') {
                next_char();
                this->token_buffer.emplace_back(TokenType::TOKEN_LESS_EQUAL, this->current_index - 1,
                                                this->current_index);
                break;
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_LESS, this->current_index, this->current_index);
            break;
        case '>':
            if (peek() == '=') {
                next_char();
                this->token_buffer.emplace_back(TokenType::TOKEN_GREATER_EQUAL, this->current_index - 1,
                                                this->current_index);
                break;
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_GREATER, this->current_index, this->current_index);
            break;
        case '!':
            if (peek() == '=') {
                next_char();
                this->token_buffer.emplace_back(TokenType::TOKEN_NOT_EQUAL, this->current_index - 1,
                                                this->current_index);
                break;
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_NOT, this->current_index, this->current_index);
            break;
        case '"': {
            u64         start = this->current_index;
            std::string str   = std::string();

            next_char();
            while (current_index < this->file_data->data_length && this->file_data->data[current_index] != '"') {
                // skip back slash and accept next char
                if (this->file_data->data[current_index] == '\\') {
                    str.append(std::string(1, this->file_data->data[current_index]));
                    next_char();
                }

                str.append(std::string(1, this->file_data->data[current_index]));
                next_char();
            }
            this->token_buffer.emplace_back(TokenType::TOKEN_STRING_LITERAL, start, this->current_index);
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            // number literals
            // will keep consuming until we hit a space, tab or newline
            u64         start = this->current_index;
            std::string str   = std::string();

            // keep doing next char until while there is data left it is not a delimiter but it can be a .
            while (this->current_index < this->file_data->data_length &&
                   (!is_delim(this->file_data->data[this->current_index]) ||
                    this->file_data->data[this->current_index] == '.')) {
                str.append(std::string(1, this->file_data->data[this->current_index]));
                next_char();
            }

            this->current_index--; // it will be iterated once after this

            this->token_buffer.emplace_back(TokenType::TOKEN_NUMBER_LITERAL, start, this->current_index);
        } break;
        default: {
            i32  word_start = this->current_index;
            auto word       = get_word();

            ASSERT(word.length() > 0);

            // check keywords
            if (compare_string(word, "let")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_LET, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "fn")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_FN, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "return")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_RETURN, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "struct")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_STRUCT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "new")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_NEW, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "continue")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_CONTINUE, word_start,
                                                (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "for")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_FOR, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "if")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_IF, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "else")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_ELSE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "and")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_AND, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "or")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_OR, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "true")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_TRUE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "false")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_FALSE, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "null")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_NULL, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "zero")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_ZERO, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "break")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_BREAK, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "match")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_MATCH, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "import")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_IMPORT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "print")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_PRINT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "assert")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_ASSERT, word_start, (word_start - 1) + word.length());
                continue;
            }

            if (compare_string(word, "while")) {
                this->token_buffer.emplace_back(TokenType::TOKEN_WHILE, word_start, (word_start - 1) + word.length());
                continue;
            }

            // must be an identifier
            this->token_buffer.emplace_back(TokenType::TOKEN_IDENTIFIER, word_start, (word_start - 1) + word.length());
        } break;
        }
    }

    return new CompilationUnit(this->file_data, this->token_buffer);
}

void Lexer::next_char() {
    this->current_index++;
}

char Lexer::peek() {
    return this->file_data->data[this->current_index + 1];
}

std::string Lexer::get_word() {
    std::string word = std::string();
    while (this->current_index < this->file_data->data_length &&
           !is_delim(this->file_data->data[this->current_index])) {
        word.append(1, this->file_data->data[this->current_index]);
        next_char();
    }

    this->current_index--; // it will be iterated once after this
    return word;
}

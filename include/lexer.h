#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

namespace liam {

    std::vector<char> extract_chars(const char* path) {
        auto vec = std::vector<char>();

        std::ifstream file;  
        file.open(path);
    
        char letter;
        while (file >> letter)
        {
            vec.push_back(letter);
        }

        file.close();
        return vec;
    }

    enum TokenType {
        INT_LITERAL,
        PLUS
    };

    struct Token
    {
        TokenType type;
        std::string string;

    };
    
    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        os << token.type << " : " << token.string << "\n";
        return os;
    }

    struct Lexer
    {
        std::vector<Token> tokens;

        Lexer() {
            tokens = std::vector<Token>();
        }

        void lex(const char* path) {
            auto chars = extract_chars(path);
            for(auto& c : chars) {
                switch (c)
                {
                    case ' ':
                    case '\r':
                    case '\n':
                    case '\t':
                    break;
                    case '+':
                        tokens.push_back(Token{TokenType::PLUS, "+"});
                        break;
                    default:
                        if(std::isdigit(c)) {
                            tokens.push_back(Token{TokenType::INT_LITERAL, std::string(1, c)});
                        }
                    break;
                }
            }
        }
    };
}
